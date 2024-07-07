#include <stdio.h>
#include <assert.h>
#include "pseudo_malloc.h"
#include <sys/mman.h>

void* pseudo_malloc(BuddyAllocator* alloc, int size){
    if (size <0){
        printf("\nErrore: Dimensione non Valida (<0)\n");
        return NULL;
    }
    if (size == 0){
        printf("\nErrore: Non è possibile allocare 0 byte\n");
        return NULL;
    }
    if (size >= THRESHOLD){
        int memory_size = size + sizeof(int); // per tenere in memoria la grandezza
        void *p = mmap(NULL,memory_size,PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (p == MAP_FAILED){
            printf("\nErrore: mmap failed");
            return NULL;
        }
        ((int*)p)[0] = memory_size;
        printf("\nMemoria allocata correttamente con mmap. indirizzo: %p, dimensione: %d \n", p+sizeof(int),memory_size);
        return (void *)(p + sizeof(int));
    }
    else {
        void* p = BuddyAllocator_malloc(alloc, size);
        if (!p) {
            printf("\nErrore: memoria del buddy allocator piena");
            return NULL;
        }
        else return p;
    }
}

void psuedo_free(BuddyAllocator* alloc, void* mem){
    if (!mem){
        printf("\nErrore: Memoria da liberare vuota. Puntatore fornito: NULL\n");
        return;
    }
    int* p = (int*) mem;
    int size = *(--p);
    if (size >= THRESHOLD){
        int ret = munmap((void*)p,size);
        if (ret != 0){
            printf("\nErrore: munmap failed");
            return;
        }
        printf("\nMemoria deallocata correttamente con munmap\n");
    }
    else{
        BuddyAllocator_free(alloc,mem);
    }
}