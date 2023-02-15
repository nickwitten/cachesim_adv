#include "cachesim.hpp"
#include <stdlib.h>
#include <stdio.h>

/**
 * The use of virtually indexed physically tagged caches limits 
 *      the total number of sets you can have.
 * If the user selected configuration is invalid for VIPT
 *      Update config->s to reflect the minimum value for S (log2 number of ways)
 * If the user selected configuration is valid
 *      do not modify it.
 * TODO: You're responsible for completing this routine
*/

void legalize_s(sim_config_t *config) {
    if (config->c - config->s > config->p) {
        config->s = config->c - config->p;
    }
}


// Valid bit is not included because the allocation of this structure indicates
// the corresponding block is in the cache
struct tag {
    char dirty;
    uint64_t value;
    struct tag *next;
};

struct translation {
    uint64_t vpn;
    uint64_t pfn;
    char valid;
    struct translation *next;
    struct translation *prev;
    // Parent is corresponding translation in hwivpt for tlb, not used by hwivpt
    struct translation *parent;
};

struct translation_storage {
    struct translation *mru;
    struct translation *lru;
} tlb, hwivpt;
// All of the translations will be allocated in a block at these locations
struct translation *tlb_translations;
struct translation *hwivpt_translations;

struct set {
    struct tag *mru;
};

struct tag_store {
    struct set **sets;
} tag_store;

int num_ways;
int num_sets;
int num_pages;
int num_tlb_entries;
bool vipt;
int s;
int m;
uint64_t index_mask;
uint64_t index_position;
uint64_t tag_mask;
uint64_t tag_position;
uint64_t offset_mask;
uint64_t offset_position;
uint64_t vpn_mask;
uint64_t vpn_position;


struct translation *initialize_translation_storage(struct translation_storage *store, uint64_t size) {
        // unlike cache, allocate all tlb and hwivpt entries
        struct translation *translations_base = (struct translation *)calloc(size, sizeof(struct translation));
        for (uint64_t i = 0; i < (size - 1); i++) {
            translations_base[i].pfn = i;
            translations_base[i].next = &(translations_base[i + 1]);
            translations_base[i + 1].prev = &(translations_base[i]);
        }
        store->mru = translations_base;
        store->lru = &(translations_base[size - 1]);
        store->lru->pfn = size - 1;
        return translations_base;
}

struct translation *search_for_translation(struct translation_storage *store, uint64_t vpn) {
    struct translation *mapping = store->mru;
    while (mapping != 0) {
        if (mapping->vpn == vpn && mapping->valid) {
            return mapping;
        }
        mapping = mapping->next;
    }
    return 0;
}

void update_translation_mru(struct translation_storage *store, struct translation *mapping) {
    if (mapping == 0) {
        return;
    }
    if (mapping->prev == 0) {
        // Already in the MRU position
        return;
    }
    // Remove from current position
    mapping->prev->next = mapping->next;
    if (mapping->next == 0) {
        // Was at the LRU position
        store->lru = mapping->prev;
    } else {
        mapping->next->prev = mapping->prev;
    }
    // Place at mru
    store->mru->prev = mapping;
    mapping->next = store->mru;
    mapping->prev = 0;
    store->mru = mapping;
}


void insert_translation(struct translation_storage *store, int64_t vpn, int64_t pfn, struct translation *parent) {
    struct translation *victim = store->lru;
    victim->valid = 1;
    victim->vpn = vpn;
    victim->pfn = pfn;
    victim->parent = parent;
    update_translation_mru(store, victim);
}

int64_t search_tlb(uint64_t addr) {
    int64_t vpn = (addr & vpn_mask) >> vpn_position;
    int64_t pfn = -1;
    struct translation *tlb_mapping = search_for_translation(&tlb, vpn);
    if (tlb_mapping == 0) {
        // Translation wasn't found
        return -1;
    }
    pfn = tlb_mapping->pfn;
    // Update lru stack in both tlb and hwivpt
    update_translation_mru(&tlb, tlb_mapping);
    update_translation_mru(&hwivpt, tlb_mapping->parent);
    return pfn;
}

int64_t search_hwivpt(uint64_t addr) {
    int64_t vpn = (addr & vpn_mask) >> vpn_position;
    int64_t pfn = -1;
    struct translation *hwivpt_mapping = search_for_translation(&hwivpt, vpn);
    if (hwivpt_mapping == 0) {
        // Translation wasn't found
        return -1;
    }
    pfn = hwivpt_mapping->pfn;
    // Insert translation into tlb
    // Update lru stack in both tlb and hwivpt
    update_translation_mru(&hwivpt, hwivpt_mapping);
    insert_translation(&tlb, vpn, pfn, hwivpt_mapping);
    return pfn;
}

int64_t page_fault_handler(uint64_t addr) {
    int64_t vpn = (addr & vpn_mask) >> vpn_position;
    insert_translation(&hwivpt, vpn, hwivpt.lru->pfn, 0);
    int64_t pfn = hwivpt.mru->pfn;
    insert_translation(&tlb, vpn, pfn, hwivpt.mru);
    return pfn;
}

void flush_cache(sim_stats_t *stats) {
    for (int i = 0; i < num_sets; i++) {
        struct tag *active_way = tag_store.sets[i]->mru;
        while (active_way != 0) {
            if (stats) {
                if (active_way->dirty) {
                    stats->cache_flush_writebacks++;
                }
            }
            struct tag *next = active_way->next;
            free(active_way);
            active_way = next;
        }
        tag_store.sets[i]->mru = 0;
    }
}

void configure_user_setup(sim_config_t *config) {
    s = config->s;
    m = config->m;
    num_ways = 1 << config->s;
    // cache size in bytes divided by (block size in bytes times blocks per set)
    num_sets = 1 << (config->c - (config->b + config->s));
    if (config->vipt) {
        vipt = true;
        num_pages = 1 << config->m;
        num_tlb_entries = 1 << config->t;
    }
}

void allocate_l1(void) {
    // Allocate a block of memory to store all of the sets
    tag_store.sets = (struct set**)calloc(num_sets, sizeof(struct set*));
    // allocate sets and store pointers in the array of sets
    for (int i = 0; i < num_sets; i++) {
        tag_store.sets[i] = (struct set*)calloc(1, sizeof(struct set));
    }
}

void configure_bit_tools(sim_config_t *config) {
    // create address masks and positions
    offset_position = 0;
    offset_mask = (1 << config->b) - 1;

    index_position = config->b;
    index_mask = ((1 << (config->c - config->s)) - 1) & ~offset_mask;

    tag_position = config->c - config->s;
    tag_mask = ~0 & ~(offset_mask | index_mask);
    if (vipt) {
        vpn_mask = ~((1 << config->p) - 1);
        vpn_position = config->p;
    }
}



/**
 * Subroutine for initializing the cache simulator. You many add and initialize any global or heap
 * variables as needed.
 * TODO: You're responsible for completing this routine
 */

void sim_setup(sim_config_t *config) {
    configure_user_setup(config);
    allocate_l1();
    configure_bit_tools(config);
    if (config->vipt) {
        tlb_translations = initialize_translation_storage(&tlb, num_tlb_entries);
        hwivpt_translations = initialize_translation_storage(&hwivpt, num_pages);
    }
}


/**
 * Subroutine that simulates the cache one trace event at a time.
 * TODO: You're responsible for completing this routine
 */
void sim_access(char rw, uint64_t addr, sim_stats_t* stats) {
    stats->accesses_l1++;
    uint64_t index = (addr & index_mask) >> index_position;
    stats->array_lookups_l1++;
    struct set *active_set = tag_store.sets[index];
    struct tag *active_way;
    int64_t pfn = -1;

    if (vipt) {
        stats->accesses_tlb++;
        pfn = search_tlb(addr);
        if (pfn < 0) {
            // Translation not in TLB
            stats->misses_tlb++;

            stats->accesses_hw_ivpt++;
            pfn = search_hwivpt(addr);
            if (pfn < 0) {
                stats->misses_hw_ivpt++;
            } else {
                stats->hits_hw_ivpt++;
                addr = (pfn << vpn_position) | (addr & ~vpn_mask);
            }

        } else {
            stats->hits_tlb++;
            addr = (pfn << vpn_position) | (addr & ~vpn_mask);
        }
    }

    int hit_flag = 0;
    uint64_t tag = (addr & tag_mask) >> tag_position;
    if (!vipt || pfn >= 0) {
        active_way = active_set->mru;
        struct tag *prev_way = 0;
        stats->tag_compares_l1 += num_ways;
        while (active_way != 0) {
            if (tag == active_way->value) {
                // HIT!!
                stats->hits_l1++;
                hit_flag = 1;
                // Remove from current position
                if (prev_way == 0) {
                    // Hit on MRU
                    active_set->mru = active_way->next;
                } else {
                    prev_way->next = active_way->next;
                }
                break;
            }
            prev_way = active_way;
            active_way = active_way->next;
        }
        if (!hit_flag) {
            stats->misses_l1++;
        }
    }

    if (vipt && pfn < 0) {
        // Page fault!!
        flush_cache(stats);
        pfn = page_fault_handler(addr);
        addr = (pfn << vpn_position) | (addr & ~vpn_mask);
        tag = (addr & tag_mask) >> tag_position;
        stats->misses_l1++;
    }

    if (!hit_flag) {
        // Allocate a new tag
        active_way = (struct tag *)calloc(1, sizeof(struct tag));
        active_way->value = tag;
    }

    if (rw == 'W') {
        stats->writes++;
        active_way->dirty = 1;
    } else if (rw == 'R') {
        stats->reads++;
    }

    // Move to MRU position
    active_way->next = active_set->mru;
    active_set->mru = active_way;

    // Purge the LRU if the set is too large
    active_way = active_set->mru;
    int i = 1;
    while (active_way != 0) {
        if (i == num_ways) {
            struct tag *victim_tag = active_way->next;
            if (victim_tag != 0) {
                stats->writebacks_l1 += (victim_tag->dirty) ? 1 : 0;
                free(victim_tag);
                active_way->next = 0;
            }
        }
        active_way = active_way->next;
        i++;
    }
}

/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * TODO: You're responsible for completing this routine
 */
void sim_finish(sim_stats_t *stats) {
    stats->hit_ratio_l1 = (double)stats->hits_l1 / stats->accesses_l1;
    stats->miss_ratio_l1 = (double)stats->misses_l1 / stats->accesses_l1;
    double tag_compare_time = L1_TAG_COMPARE_TIME_CONST + s * L1_TAG_COMPARE_TIME_PER_S;
    double hit_time = L1_ARRAY_LOOKUP_TIME_CONST + tag_compare_time;
    double miss_penalty = DRAM_ACCESS_PENALTY;

    if (vipt) {
        stats->hit_ratio_tlb = (double)stats->hits_tlb / stats->accesses_tlb;
        stats->miss_ratio_tlb = (double)stats->misses_tlb / stats->accesses_tlb;
        stats->hit_ratio_hw_ivpt = (double)stats->hits_hw_ivpt / stats->accesses_hw_ivpt;
        stats->miss_ratio_hw_ivpt = (double)stats->misses_hw_ivpt / stats->accesses_hw_ivpt;
        double hwivpt_penalty = (1 + HW_IVPT_ACCESS_TIME_PER_M * m) * DRAM_ACCESS_PENALTY;
        hit_time = (L1_ARRAY_LOOKUP_TIME_CONST +
            stats->hit_ratio_tlb * tag_compare_time +
            stats->miss_ratio_tlb * (hwivpt_penalty + tag_compare_time * stats->hit_ratio_hw_ivpt));
    }

    stats->avg_access_time = hit_time + stats->miss_ratio_l1 * miss_penalty;

    // Free the tag store
    flush_cache(0);
    for (int i = 0; i < num_sets; i++) {
        free(tag_store.sets[i]);
    }
    if (vipt) {
        // Free the virtual address translations
        free(tlb_translations);
        free(hwivpt_translations);
    }
}
