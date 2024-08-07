#include "buddy_allocator.h"
#include <stdio.h>
#include "pseudo_malloc.h"

#define BITMAP_SIZE 1<<14
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

    void *blocks[50];

    // test con size non valide
    printf("\n\nTEST ERRORI SULLA SIZE\n");
    pseudo_malloc(&alloc,0);
    pseudo_malloc(&alloc,-1);

    //alloco vari blocchi di dimensioni variabile 
    //alcuni sono allocati con mmap, altri con il buddy allocator
    printf("\n\nTEST ALLOCAZIONE BLOCCHI DI DIMENSIONE VARIABILE\n");
    for (int i = 0; i<20; i++){
      void* p = pseudo_malloc(&alloc,1<<i);
      blocks[i] = p;
    }
    // deallocazione
    printf("\n\nTEST DEALLOCAZIONE BLOCCHI DI DIMENSIONE VARIABILE\n");
    for (int i = 0; i<20; i++){
      pseudo_free(&alloc, &blocks[i]);
    } 
    // dealloco una seconda volta (double free)
    printf("\n\nTEST DOUBLE FREE\n");
    for (int i = 0; i<20; i++){ // sui blocchi deallocati dal buddy allocator ci sarà l'avvertimento di double free
      pseudo_free(&alloc, &blocks[i]);  // i blocchi allocati da mmap invece saranno stati messi a NULL per evitare
                                        // segmentation faults
    }

    //allocazione blocchi grandi 
    printf("\n\nTEST ALLOCAZIONE BLOCCHI DI GRANDI DIMENSIONI\n");
    void* p1 = pseudo_malloc(&alloc,1000000000);
    void* p2 = pseudo_malloc(&alloc,2000000000);
    void* p3 = pseudo_malloc(&alloc,2147483647);

    printf("\n\nTEST DEALLOCAZIONE BLOCCHI DI GRANDI DIMENSIONI\n");
    pseudo_free(&alloc, &p1);
    pseudo_free(&alloc, &p2);
    pseudo_free(&alloc, &p3);

}