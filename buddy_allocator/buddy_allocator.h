#pragma once
#include "pool_allocator.h"
#include "linked_list.h"

#define MAX_LEVELS 16

// one entry of the buddy list
typedef struct BuddyListItem {
  ListItem list;
  int idx;   // tree index
  int level; // Livello a cui sta
  char* start; // start of memory del suo blocco
  int size; //grandezza del blocco
  struct BuddyListItem* buddy_ptr;
  struct BuddyListItem* parent_ptr;
} BuddyListItem;


typedef struct  {
  ListHead free[MAX_LEVELS]; // ogni livello ha la sua free list
  int num_levels;   // num di livelli
  PoolAllocator list_allocator; // allocatore (SLAB) per le free list
  char* memory; // the memory area to be managed (TOTALE)
  int min_bucket_size; // the minimum page of RAM that can be returned
} BuddyAllocator;


// computes the size in bytes for the buffer of the allocator
int BuddyAllocator_calcSize(int num_levels);


// initializes the buddy allocator, and checks that the buffer is large enough
void BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         char* buffer,
                         int buffer_size,
                         char* memory,
                         int min_bucket_size);


// returns (allocates) a buddy at a given level.
// side effect on the internal structures
// 0 id no memory available
BuddyListItem* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level);

// releases an allocated buddy, performing the necessary joins
// side effect on the internal structures
void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, BuddyListItem* item);

//allocates memory
void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size);

//releases allocated memory
void BuddyAllocator_free(BuddyAllocator* alloc, void* mem);
