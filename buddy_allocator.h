#pragma once
#include "bit_map.h"

#define MAX_LEVELS 30 // 2^30 = 1GB

typedef struct {
    int num_levels; 
    int memory_size;
    char* memory; // the memory area to be managed
    int min_bucket_size; // the minimum page of RAM that can be returned
    BitMap bitmap;
} BuddyAllocator;

// initializes the buddy allocator, and checks that the buffer is large enough
int BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         char* memory,
                         int memory_size,
                         char* bitmap_buffer,
                         int bitmap_buffer_size,
                         int min_bucket_size);

