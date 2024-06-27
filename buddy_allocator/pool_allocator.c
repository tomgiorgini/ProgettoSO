#include "pool_allocator.h"

static const int NullIdx=-1;
static const int DetachedIdx=-2;

static const char* PoolAllocator_strerrors[]=
  {"Success",
   "NotEnoughMemory",
   "UnalignedFree",
   "OutOfRange",
   "DoubleFree",
   0
  };

const char* PoolAllocator_strerror(PoolAllocatorResult result) {
  return PoolAllocator_strerrors[-result];
}

// Inizializzazione del PoolAllocator
PoolAllocatorResult PoolAllocator_init(PoolAllocator* a,
		       int item_size,
		       int num_items,
		       char* memory_block,
		       int memory_size) {

  // we first check if we have enough memory
  // for the bookkeeping
  int requested_size= num_items*(item_size+sizeof(int));
  if (memory_size<requested_size)
    return NotEnoughMemory;

  a->item_size=item_size;
  a->size=num_items;
  a->buffer_size=item_size*num_items;
  a->size_max = num_items;
  
  a->buffer = memory_block; // the upper part of the buffer is used as memory
  a->free_list = (int*)(memory_block+item_size*num_items); // the lower part is for bookkeeping

  // now we populate the free list by constructing a linked list
  for (int i=0; i<a->size-1; ++i){
    a->free_list[i]=i+1;  //ogni elemento viene collegato a quello dopo
  }
  // set the last element to "NULL" 
  a->free_list[a->size-1] = NullIdx;
  a->first_idx=0;
  return Success;
}

void* PoolAllocator_getBlock(PoolAllocator* a) {
  // se non ci sono blocchi liberi
  if (a->first_idx==-1)
    return 0;

  // si prende e rimuove il primo blocco libero dalla free list
  int detached_idx = a->first_idx;
  a->first_idx=a->free_list[a->first_idx]; //il primo blocco libero è il blocco puntato dal blocco che stiamo rilevando
  --a->size; // si decrementa la size di 1
  
  a->free_list[detached_idx]=DetachedIdx; // indirizzo del blocco nella free list
  
  char* block_address=a->buffer+(detached_idx*a->item_size);  // indirizzo del blocco in memoria
  return block_address;
}

PoolAllocatorResult PoolAllocator_releaseBlock(PoolAllocator* a, void* block_){
  // abbiamo l'indirizzo in memoria, dobbiamo trovare l'index
  char* block=(char*) block_;
  int offset=block - a->buffer; // si calcola l'offset rispetto all'inizio del buffer

  //sanity check, verifichiamo che sia allineato con i blocchi
  if (offset%a->item_size)
    return UnalignedFree;

  int idx=offset/a->item_size;  // indice del blocco nella free list

  //sanity check, se siamo dentro alla lista
  if (idx<0 || idx>=a->size_max)
    return OutOfRange;

  //check sulla natura del blocco, è già stato liberato?
  if (a->free_list[idx]!=DetachedIdx)
    return DoubleFree;

  //Passati i controlli, si fa inserimento in testa
  a->free_list[idx]=a->first_idx;
  a->first_idx=idx;
  ++a->size; 
  return Success;
}
