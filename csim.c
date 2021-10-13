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
    int dirty_bit;
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

csim_stats_t *stats;
stats->hits = 0;
stats->misses = 0;
stats->evictions = 0;
stats->dirty_bytes = 0;
stats->dirty_evictions = 0;

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
            printf("Malloc for line failed\n");
        }
        for (int j = 0; j < E; j++) {
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].dirty_bit = 0;
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
    int s, E, b = 0;
    char* tracefile;

    // parse arguments
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
                tracefile = optarg;
                break;
            case 'h':
            default:
                print_uasge();
                break;
        }
    }
    
    if (s <= 0 || E <= 0 || b <= 0 || t == NULL) {
        printf("Invalid input!\n")
        return -1;
    }

    FILE *pFile;
    pFile = fopen(tracefile, "r");
    if (pFile == NULL) {
        printf("Open file error\n")
        return -1;
    }
    char access_type;
    unsigned long address;
    int size;
    while (fscanf(pFile, "%c %lx,%d", &access_type, &address, &size) > 0) {
        if (access_type == "L") {
            // load data
        } else if (access_type == "S") {
            // store data
        }
    }
    fclose(pFile);

    cache_t cache = cache_init(s, E, b);
    cache_free(cache);

    printSummary(stats);
    return 0;
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
