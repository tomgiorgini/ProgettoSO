#include <stdio.h>
#include <assert.h>
#include "pseudo_malloc.h"
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

// pseudo_malloc che, a seconda della dimensione del blocco di memoria da allocare (se maggiore o minore di 1024 bytes)
// decide se allocare con mmap o con il buddy allocatore che viene passato come argomento

// è necessario salvare nel blocco da allocare la grandezza del blocco, cosicchè la psuedo_free possa sapere
// se deallocare il blocco con munmap (se è stato allocato con mmap) o con la free del buddy allocator
void* pseudo_malloc(BuddyAllocator* alloc, int size){
    // controlli sulla size
    if (size <0){
    printf("\nErrore di malloc: Invalid Size (<0)\n");
        return NULL;
    }
    if (size == 0){
        printf("\nErrore di malloc: Impossibile allocare 0 byte\n");
        return NULL;
    }
    // se la dimensione è pari a 1/4*PAGE_SIZE (1/4 * 4096 = 1024) si usa
    // la syscall mmap per allocare la memoria 
    if (size >= THRESHOLD){ // in questo caso si hanno 4 bytes di overhead
        printf("\nAllocazione da eseguire con mmap, size: %d\n",size);
        int memory_size = size + sizeof(int); // per tenere in memoria la grandezza 
        void *p = mmap(NULL,memory_size,PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // controllo sulla corretta allocazione 
        if (p == MAP_FAILED){
            printf("Errore di malloc: mmap failed with error %s",strerror(errno));
            return NULL;
        }
        ((int*)p)[0] = memory_size; // si salva la grandezza (originale) nel blocco allocato
        printf("Allocazione Riuscita: indirizzo: %p, di dimensioni: %d\n", p+sizeof(int),memory_size);
        return (void *)(p + sizeof(int)); // si restituisce il puntatore corretto
    }
    else { // in questo caso si hanno 8 bytes di overhead
        printf("\nAllocazione da eseguire con Buddy Allocator, size: %d",size);
        void* p = BuddyAllocator_malloc(alloc, size);
        if (!p) { // errore di allocazione (la printf sta nel buddy allocator)
            return NULL;
        }
        else return p;
    }
}

// pseudo free che estrapola dal blocco la sua dimensione (senza contare l'overhead)
// per capire se è stato allocato con mmap o buddy allocator.
// a questo punto dealloca la memoria con il corrispettivo metodo (munmap o buddyallocator_free)
void pseudo_free(BuddyAllocator* alloc, void** mem){
    if (!(*mem)){
        printf("\nErrore di deallocazione: Memoria da liberare vuota. Puntatore fornito: NULL\n");
        return;
    }
    int* p = (int*)(*mem);
    int size = *(--p);
    if (size >= THRESHOLD){
        printf("\nDeallocazione da eseguire con munmap");
        int ret = munmap((void*)p,size);
        if (ret != 0){
            printf("\nErrore di deallocazione: munmap failed\n");
            return;
        }
        printf("\nDeallocazione riuscita: Blocco di memoria all'indice %p libero\n", mem);
        *mem = NULL; // per evitare double free => segmentation faults
    }
    else{
        printf("\nDeallocazione da eseguire con Buddy Allocator");
        BuddyAllocator_free(alloc,*mem);
    }
}