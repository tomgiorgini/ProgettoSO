#pragma once
#include "bit_map.h"

#define MAX_LEVELS 20 // (con 1 MB)

typedef struct {
    char* memory; // the memory area to be managed
    int memory_size;
    int num_levels;
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

void* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level,int size);

void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int bit,void* mem);

void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);

void update_parent(BitMap *bitmap, int bit, int value);

void update_child(BitMap *bitmap, int  bit, int value);

void merge(BitMap *bitmap, int bit);
