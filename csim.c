/**
 * @file csim.c
 * @brief A cache simulator
 *
 * This program implements a cache simulator that simulates the behavior
 * of a cache memory with arbitary size and associativity.
 *
 * @author Yujia Wang <yujiawan@andrew.cmu.edu>
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cachelab.h"

/**
 * @brief Line structure of a set
 */
typedef struct {
    int valid;     /* valid bit set 1 if line has data loaded */
    int dirty_bit; /* dirty bit set 1 if payload has been modified, but has not
                      written back to memory */
    unsigned long tag; /* tag of line */
    unsigned long
        LRU_counter; /* LRU counter, evict the block with max LRU counter */
} line_t;

/**
 * @brief Set structure of a cache
 */
typedef struct {
    line_t *lines; /* pointer to lines of a set */
} set_t;

/**
 * @brief Cache structure with parameters
 */
typedef struct {
    int s;       /* Number of set index bits */
    int E;       /* Associativity (number of lines per set) */
    int b;       /* Number of block bits */
    set_t *sets; /* pointer to sets of a cache */
} cache_t;

unsigned long hit = 0;             /* number of hits */
unsigned long miss = 0;            /* number of misses */
unsigned long eviction = 0;        /* number of evictions */
unsigned long dirty_bytes = 0;     /* number of dirty bytes in cache at end */
unsigned long dirty_evictions = 0; /* number of dirty bytes evicted */

/**
 * @brief Initialize a new cache
 *
 * @param[in] s Number of set index bits
 * @param[in] E Associativity (number of lines per set)
 * @param[in] b Number of block bits
 *
 * @return The new cache, or NULL if memory allocation failed
 */
cache_t *cache_init(int s, int E, int b) {
    cache_t *cache = (cache_t *)malloc(sizeof(cache_t));
    if (cache == NULL) {
        printf("Malloc for cache failed\n");
        return NULL;
    }
    cache->s = s;
    cache->E = E;
    cache->b = b;

    int S = 1 << s;
    cache->sets = (set_t *)malloc(sizeof(set_t) * (unsigned long)S);
    if (cache->sets == NULL) {
        printf("Malloc for set failed\n");
        free(cache);
        return NULL;
    }

    for (int i = 0; i < S; i++) {
        cache->sets[i].lines =
            (line_t *)malloc(sizeof(line_t) * (unsigned long)E);
        if (cache->sets[i].lines == NULL) {
            printf("Malloc for line failed\n");
            free(cache->sets);
            free(cache);
            return NULL;
        }

        for (int j = 0; j < E; j++) {
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].dirty_bit = 0;
            cache->sets[i].lines[j].tag = 0;
            cache->sets[i].lines[j].LRU_counter = 0;
        }
    }
    return cache;
}

/**
 * @brief Free all memory used by a cache
 *
 * @param[in] cache the cache to free
 */
void cache_free(cache_t *cache) {
    int S = 1 << cache->s;
    for (int i = 0; i < S; i++) {
        free(cache->sets[i].lines);
    }
    free(cache->sets);
    free(cache);
}

/**
 * @brief Helper function to print usage info
 */
void print_usage() {
    printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n"
           "-h: Optional help flag that prints usage info\n"
           "-v: Optional verbose flag that displays trace info\n"
           "-s <s>: Number of set index bits (S = 2^s is the number of sets)\n"
           "-E <E>: Associativity (number of lines per set)\n"
           "-b <b>: Number of block bits (B = 2^b is the block size)\n"
           "-t <tracefile>: Name of the memory trace to replay");
}

/**
 * @brief Add LRU counter of unused lines
 */
void add_LRU_counter(set_t *set_access, int begin_index, int end_index) {
    for (int i = begin_index; i < end_index; i++) {
        set_access->lines[i].LRU_counter++;
    }
}

/**
 * @brief A cache simulator to simulate the behavior of a cache memory with data
 * load and store
 */
void cache_sim(char access_type, set_t *set_access, unsigned long tag, int E,
               int b, bool verbose) {
    unsigned long B = (unsigned long)(1 << b);
    int hit_flag = 0;
    int hit_index = 0;
    for (int i = 0; i < E; i++) {
        line_t *line_access = &(set_access->lines[i]);
        if (line_access->tag == tag && line_access->valid == 1) {
            if (verbose) {
                printf("hit\n");
            }
            hit++;
            line_access->LRU_counter = 0;
            hit_flag = 1;
            hit_index = i;
            break;
        } else {
            line_access->LRU_counter++;
        }
    }
    if (hit_flag == 1) {
        add_LRU_counter(set_access, hit_index + 1, E);
        if (access_type == 'S' && set_access->lines[hit_index].dirty_bit == 0) {
            set_access->lines[hit_index].dirty_bit = 1;
            dirty_bytes += B;
        }
        return;
    }

    if (verbose) {
        printf("miss");
    }
    miss++;
    int flag = 0;
    int index = 0;
    for (int i = 0; i < E; i++) {
        line_t *line_access = &(set_access->lines[i]);
        if (line_access->valid == 0) {
            if (verbose) {
                printf("\n");
            }
            line_access->valid = 1;
            line_access->tag = tag;
            line_access->LRU_counter = 0;
            flag = 1;
            index = i;
            break;
        } else {
            line_access->LRU_counter++;
        }
    }
    if (flag == 1) {
        add_LRU_counter(set_access, index + 1, E);
        if (access_type == 'S') {
            set_access->lines[index].dirty_bit = 1;
            dirty_bytes += B;
        }
        return;
    }

    if (verbose) {
        printf(" eviction\n");
    }
    eviction++;
    int evict_index = 0;
    unsigned long max_counter = set_access->lines[0].LRU_counter;
    for (int i = 1; i < E; i++) {
        line_t *line_access = &(set_access->lines[i]);
        if (line_access->LRU_counter > max_counter) {
            evict_index = i;
        }
    }
    set_access->lines[evict_index].tag = tag;
    set_access->lines[evict_index].LRU_counter = 0;
    add_LRU_counter(set_access, 0, evict_index);
    add_LRU_counter(set_access, evict_index + 1, E);

    if (set_access->lines[evict_index].dirty_bit == 0 && access_type == 'S') {
        set_access->lines[evict_index].dirty_bit = 1;
        dirty_bytes += B;
        return;
    }

    if (set_access->lines[evict_index].dirty_bit == 1) {
        if (access_type == 'L') {
            set_access->lines[evict_index].dirty_bit = 0;
            dirty_bytes -= B;
            dirty_evictions += B;
        }
        if (access_type == 'S') {
            set_access->lines[evict_index].dirty_bit = 1;
            dirty_evictions += B;
        }
    }
}

int main(int argc, char *argv[]) {
    int s, E, b = 0;
    bool verbose = false;
    char *tracefile;

    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            tracefile = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case 'h':
        default:
            print_usage();
        }
    }

    if (s < 0 || E <= 0 || b < 0 || tracefile == NULL) {
        printf("Invalid input!\n");
        return -1;
    }

    cache_t *cache = cache_init(s, E, b);
    FILE *pFile;
    pFile = fopen(tracefile, "r");
    if (pFile == NULL) {
        printf("Open file error\n");
        return -1;
    }
    char access_type;
    unsigned long address;
    int size;
    while (fscanf(pFile, "%c %lx,%d\n", &access_type, &address, &size) > 0) {
        unsigned long set_index = (address >> b) & ((1 << s) - 1);
        unsigned long tag = address >> (s + b);
        set_t *set_access = &(cache->sets[set_index]);
        if (verbose) {
            printf("%c %lx,%d ", access_type, address, size);
        }
        if ((access_type == 'L') || (access_type == 'S')) {
            cache_sim(access_type, set_access, tag, E, b, verbose);
        } else {
            printf("Tracefile error\n");
            return -1;
        }
    }
    fclose(pFile);
    cache_free(cache);

    csim_stats_t stats;
    stats.hits = hit;
    stats.misses = miss;
    stats.evictions = eviction;
    stats.dirty_bytes = dirty_bytes;
    stats.dirty_evictions = dirty_evictions;
    printSummary(&stats);
    return 0;
}
