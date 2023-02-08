#include "cachesim.hpp"

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

/**
 * Subroutine for initializing the cache simulator. You many add and initialize any global or heap
 * variables as needed.
 * TODO: You're responsible for completing this routine
 */

void sim_setup(sim_config_t *config) {
    
}

/**
 * Subroutine that simulates the cache one trace event at a time.
 * TODO: You're responsible for completing this routine
 */
void sim_access(char rw, uint64_t addr, sim_stats_t* stats) {

}

/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * TODO: You're responsible for completing this routine
 */
void sim_finish(sim_stats_t *stats) {
    
}
