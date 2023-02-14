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

}


// Valid bit is not included because the allocation of this structure indicates
// the address is in the cache
struct tag {
    char dirty;
    uint64_t value;
    struct tag *next;
};

struct set {
    struct tag *mru;
};
int num_ways;

struct tag_store {
    struct set **sets;
} tag_store;

uint64_t index_mask;
uint64_t tag_mask;
uint64_t offset_mask;
uint64_t index_position;
uint64_t tag_position;
uint64_t offset_position;

/**
 * Subroutine for initializing the cache simulator. You many add and initialize any global or heap
 * variables as needed.
 * TODO: You're responsible for completing this routine
 */

void sim_setup(sim_config_t *config) {
    // calculate the size of the tag store
    // int s = (config->s) ? config->s : 1;  // if s is 0 there is still one block per set
    // num_ways = (s) ? 1 << (s - 1) : 1;
    num_ways = 1 << config->s;
    int num_sets = 1 << (config->c - (config->b + config->s));  // cache size in bytes divided by (block size in bytes times blocks per set)
    printf("num sets: %d\n", num_sets);
    tag_store.sets = (struct set**)calloc(num_sets, sizeof(struct set*));  // Allocate a block of memory to store all of the sets
    for (int i = 0; i < num_sets; i++) {
        tag_store.sets[i] = (struct set*)calloc(1, sizeof(struct set));
    }
    // create address masks and positions
    offset_position = 0;
    offset_mask = (1 << config->b) - 1;
    printf("offset mask: %lX\n", offset_mask);
    index_position = config->b;
    index_mask = ((1 << (config->c - config->s)) - 1) & ~offset_mask;
    printf("index mask: %lX\n", index_mask);
    tag_position = config->c - config->s;
    tag_mask = ~0 & ~(offset_mask | index_mask);
    printf("tag mask: %lX\n", tag_mask);
}

/**
 * Subroutine that simulates the cache one trace event at a time.
 * TODO: You're responsible for completing this routine
 */
void sim_access(char rw, uint64_t addr, sim_stats_t* stats) {
    uint64_t index = addr & index_mask >> index_position;
    uint64_t tag = addr & tag_mask >> tag_position;
    struct set *active_set = tag_store.sets[index];
    struct tag *active_way = active_set->mru;
    struct tag *prev_way = 0;
    int hit_flag = 0;
    while (active_way != 0) {
        if (tag == active_way->value) {
            // HIT!!
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
        printf("Allocating new tag");
        // Allocate a new tag
        active_way = (struct tag *)calloc(1, sizeof(struct tag));
        active_way->value = tag;
    }
    if (rw == 'W') {
        active_way->dirty = 1;
    }
    // Move to MRU position
    active_way->next = active_set->mru;
    active_set->mru = active_way;

    // Purge the LRU if the set is too large
    active_way = active_set->mru;
    int i = 1;
    while (active_way != 0) {
        if (i == num_ways) {
            if (active_way->next != 0) {
                free(active_way->next);
            }
            active_way->next = 0;
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
}
