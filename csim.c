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
} line;

typedef struct {
    line *lines;
} set;

typedef struct {
    int s;
    int E;
    int b;
    set *sets;
} cache;

typedef struct {
    unsigned long hit;
    unsigned long miss;
    unsigned long eviction;
} result;

/**
 * 
 */
cache *cache_init(int s, int E, int b) {
    cache *new_cache = (cache *)malloc(sizeof(cache));
    if (new_cache == NULL) {
        printf("Malloc for cache failed\n");
        return NULL;
    }
    new_cache->s = s;
    new_cache->E = E;
    new_cache->b = b;

    int S = 1 << s;
    new_cache->sets = (set *)malloc(sizeof(set) * S);
    if (new_cache->sets == NULL) {
        printf("Malloc for set failed\n")
        return NULL;
    }
    for (int i = 0; i < S; i++) {
        new_cache->sets[i].lines = (line *)malloc(sizeof(line) * E);
        if (new_cache->sets[i].lines == NULL) {
            printf("Malloc for line feiled\n");
        }
        for (int j = 0; j < E; j++) {
            new_cache->sets[i].lines[j].valid = 0;
            new_cache->sets[i].lines[j].LRUcounter = 0;
        }
    }

    return new_cache;
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
    status->hits = hit;
    status->misses = miss;
    status->evictions = eviction;
    status->dirty_bytes = dirty_bytes;
    status->dirty_evictions = dirty_evictions;
    printSummary(status);

    // close file

    // free memory

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
