#include "buddy_allocator.h"
#include <stdio.h>

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
    // Testing del Buddy Allocator
    void *blocks[50];

    //Provo ad allocare 0 byte, size negativa e maggiore di 1MB
    printf("\n\nTEST ERRORI SULLA SIZE\n");
    BuddyAllocator_malloc(&alloc,0);
    BuddyAllocator_malloc(&alloc,-1);
    BuddyAllocator_malloc(&alloc,2000000);

    // alloco tutta la memoria del buddy allocator e 1 blocchi in più
    printf("\n\nALLOCO TUTTA LA MEMORIA CON 8 BLOCCHI AL LIVELLO 3 ED UN BLOCCO DI TROPPO\n");
    for (int i = 0; i<9; i++){
      void* p = BuddyAllocator_malloc(&alloc,100000);
      blocks[i] = p;
    }
    // provo ad allocare blocchi più piccoli. darà problemi
    // perchè ho riempito la memoria
    printf("\n\nTEST ALLOCAZIONE BLOCCHI SPARSI CON MEMORIA PIENA\n");
    BuddyAllocator_malloc(&alloc,1);
    BuddyAllocator_malloc(&alloc,21);
    BuddyAllocator_malloc(&alloc,76);
    BuddyAllocator_malloc(&alloc,1000);
    BuddyAllocator_malloc(&alloc,12000);

    // dealloco tutta la memoria, e provo a deallocare anche i blocchi che non sono stati deallocati
    printf("\n\nDEALLOCO TUTTA LA MEMORIA (GLI 8 BLOCCHI AL LIVELLO 3 + BLOCCO NON ALLOCATO) \n");
    for (int i = 0; i<9; i++){
      BuddyAllocator_free(&alloc,blocks[i]);
    }

    // dealloco di nuovo la memoria (double free)
    printf("\n\nDEALLOCO NUOVAMENTE (DOUBLE FREE)\n");
    for (int i = 0; i<9; i++){
      BuddyAllocator_free(&alloc,blocks[i]);
    }
    
    // allocazione di blocchi sparsi e deallocazione al contrario
    printf("\n\nTEST ALLOCAZIONE BLOCCHI SPARSI\n");
    for (int i = 0; i<20; i++){
      void* p = BuddyAllocator_malloc(&alloc,1<<i);
      blocks[i] = p;
    }

    printf("\n\nTEST DEALLOCAZIONE BLOCCHI SPARSI AL CONTRARIO\n");
    for (int i = 19; i>=0; i--){
      BuddyAllocator_free(&alloc,blocks[i]);
    }
}