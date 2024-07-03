#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

///////////////////////////////////////////////////////////
// these are trivial helpers to support you in case you want
// to do a bitmap implementation

//level of node i
int levelIdx(size_t idx){
  return (int)floor(log2(idx));
};
//index of the buddy of node i
int buddyIdx(int idx){
  if (idx&0x1){
    return idx-1;
  }
  return idx+1;
}
//parent of the node idx
int parentIdx(int idx){
  return idx/2;
}
// idx of 1st node of a level i
int firstIdx(int level){
  return (1 << level); 
}
//offset of node idx in his level
int startIdx(int idx){
    return (idx-(firstIdx(levelIdx(idx))));
}
///////////////////////////////////////////////////////////

int BuddyAllocator_init(BuddyAllocator* alloc,
                         int num_levels,
                         char* memory,
                         int memory_size,
                         char* bitmap_buffer,
                         int bitmap_buffer_size,
                         int min_bucket_size){
    // check on num_levels
    assert("Numero di Livelli Superiore al Massimo (20 -> 1MB)" && num_levels<MAX_LEVELS);

    // check on min_bucket_size
    assert("Dimensione del min_bucket size troppo piccola (<8)" && min_bucket_size>=8);
    
    // se la memoria non è una potenza di 2, verrà utilizzata la porzione di memoria
    // più grande possibile (disponibile) che è potenza di 2
    if (log2(memory_size) != floor(log2(memory_size))){
        memory_size = min_bucket_size << num_levels;
        printf("Memoria utilizzata : %d\n",memory_size);
    }

    alloc->num_levels=num_levels;
    alloc->memory=memory;
    alloc->memory_size=memory_size;
    alloc->min_bucket_size=min_bucket_size;
    
    //check e inizializzazione bitmap
    int num_bits =(1 << (num_levels+1)) - 1;
    assert("Memoria insufficiente per contenere la bitmap" && bitmap_buffer_size >= BitMap_getBytes(num_bits));

    BitMap_init(&(alloc->bitmap),num_bits,bitmap_buffer);
    printf("BUDDY ALLOCATOR CREATO\nLivelli: %d\nDimensione della Memoria: %d\nNumero di bit nella bitmap: %d\nDimensione della bitmap: %d\nDimensione Bucket Minima: %d\n", num_levels,memory_size,num_bits,BitMap_getBytes(num_bits),min_bucket_size);
    return 1;
}

void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size){
  return NULL;
}

void BuddyAllocator_free(BuddyAllocator* alloc, void* mem){

}

void merge(BitMap *bitmap, int bit){
    if (bit == 0) return; //radice

    int value = BitMap_bit(bitmap, bit);
    if (value == 1){
      printf("Impossibile effettuare il merge su un bit non libero\n");
      return;
    }
    int buddy = buddyIdx(bit);
    value = BitMap_bit(bitmap,buddy);
    if (value == 1){
      printf("Il buddy non è libero, quindi non si può fare il merge\n");
      return;
    }
    int parent = parentIdx(bit);
    BitMap_setBit(bitmap,parent,0);
    merge(bitmap,parent); // ricorsione in risalita
}

void update_parent(BitMap *bitmap, int bit, int value) {
  BitMap_setBit(bitmap,  bit,value);
  if ( bit > 0) {
    update_parent(bitmap, parentIdx(bit), value); // ricorsione in risalita
  }
}

void update_child(BitMap *bitmap, int  bit, int value) {
  if ( bit < bitmap->num_bits) { 
    BitMap_setBit(bitmap,  bit, value);
    // si fa ricorsione binaria sui figli
    update_child(bitmap,  bit * 2 + 1, value);  // sinistro
    update_child(bitmap,  bit * 2 + 2, value);  // destro
  }
}

