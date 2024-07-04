#include "buddy_allocator.h"
#include <stdio.h>

#define BITMAP_SIZE 2<<14
#define BUDDY_LEVELS 15
#define MEMORY_SIZE (1024*1024) // 1MB
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS)) // 2^20 - 2^15 = 2^5 = 32

char bitmap_buffer[BITMAP_SIZE]; 
char memory[MEMORY_SIZE];

BuddyAllocator alloc;

int main(int argc, char** argv){

    // si inizializzano i parametri
    int num_levels = BUDDY_LEVELS;
    int memory_size = MEMORY_SIZE;
    int bitmap_buffer_size = BITMAP_SIZE;
    int min_bucket_size = MIN_BUCKET_SIZE;
    int res = BuddyAllocator_init(&alloc,
                                    num_levels,
                                    memory,
                                    memory_size,
                                    bitmap_buffer,
                                    bitmap_buffer_size,
                                    min_bucket_size);
    if (res == 0){
        printf("Errore di inizializzazione del Buddy Allocator, quit\n");
        return 0;
    }
    /*
    void* t1 = BuddyAllocator_malloc(&alloc,677);
    void* t2 = BuddyAllocator_malloc(&alloc,1683);
    void* t3 = BuddyAllocator_malloc(&alloc,483);
    void* t4 = BuddyAllocator_malloc(&alloc,1683);
    void* t5 = BuddyAllocator_malloc(&alloc,3);
    BuddyAllocator_free(&alloc,t1);
    BuddyAllocator_free(&alloc,t2);
    BuddyAllocator_free(&alloc,t3);
    BuddyAllocator_free(&alloc,t4);
    BuddyAllocator_free(&alloc,t5);
    */
   for (int i = 0; i<300;i++){
    void* t1 = BuddyAllocator_malloc(&alloc,100 + 3*i);
    printf("%d\n",i);
   }
}