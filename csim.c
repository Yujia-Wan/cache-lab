/**
 * @file csim.c
 * @brief A cache simulator
 *
 * This program implements a cache simulator that simulates the behavior
 * of a cache memory with arbitary size and associativity.
 * 
 * @author Yujia Wang <yujiawan@andrew.cmu.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "cachelab.h"

#define true 1
#define false 0

typedef struct {
    int valid;
    unsigned long tag;
    int LRUcounter;
} line_t;

typedef struct {
    line_t *lines;
} set_t;

typedef struct {
    int s;
    int E;
    int b;
    set_t *sets;
} cache_t;

typedef struct {
    unsigned long hit;
    unsigned long miss;
    unsigned long eviction;
} result;

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
    cache->sets = (set_t *)malloc(sizeof(set_t) * S);
    if (cache->sets == NULL) {
        printf("Malloc for set failed\n")
        return NULL;
    }
    for (int i = 0; i < S; i++) {
        cache->sets[i].lines = (line_t *)malloc(sizeof(line_t) * E);
        if (cache->sets[i].lines == NULL) {
            printf("Malloc for line feiled\n");
        }
        for (int j = 0; j < E; j++) {
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].LRUcounter = 0;
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
        free(cache->sets[i].lines;
    }
    free(cache->sets);
    free(cache);
}

int main(int argc, char *argv[]) {
    int s = 0, E = 0, b = 0;
    int opt;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'v':
                // TODO
                break;
            case "s":
                s = atol(optarg);
                break;
            case 'E':
                E = atol(optarg);
                break;
            case 'b':
                b = atol(optarg);
                break;
            case 't':
                // TODO
                break;
            case 'h':
            default:
                print_uasge();
                break;
        }
    }

    csim_stats_t *status;
    status->hits = hits;
    status->misses = misses;
    status->evictions = evictions;
    status->dirty_bytes = dirty_bytes;
    status->dirty_evictions = dirty_evictions;
    printSummary(status);

    cache_free(cache);
}

void print_usage() {
    printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n"
           "-h: Optional help flag that prints usage info\n"
           "-v: Optional verbose flag that displays trace info\n"
           "-s <s>: Number of set index bits (S = 2^s is the number of sets)\n"
           "-E <E>: Associativity (number of lines per set)\n"
           "-b <b>: Number of block bits (B = 2^b is the block size)\n"
           "-t <tracefile>: Name of the memory trace to replay");
}
