#include "buddy_allocator.h"
#include <stdio.h>
#include "pseudo_malloc.h"

#define BITMAP_SIZE 2<<13
#define BUDDY_LEVELS 16
#define MEMORY_SIZE ((1024*1024) + 1) // 1MB
#define MIN_BUCKET_SIZE (MEMORY_SIZE>>(BUDDY_LEVELS)) // 2^20 - 2^16 = 2^4 = 16

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
    if (res == -1){
        printf("Errore di inizializzazione del Buddy Allocator, quit\n");
        return -1;
    }


    
  
}