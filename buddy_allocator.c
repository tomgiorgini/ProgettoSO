#include <stdio.h>
#include <assert.h>
#include <math.h> // for floor and log2
#include "buddy_allocator.h"

///////////////////////////////////////////////////////////
// these are trivial helpers to support you in case you want
// to do a bitmap implementation

// il nodo alla radice della bitmap è 0
//level of node i
int levelIdx(size_t idx){
  return (int)floor(log2(idx+1));
};
//index of the buddy of node i
int buddyIdx(int idx){
  if (idx == 0) //radice
    return 0;
  if (idx%2) // se è pari
    return idx-1;
  return idx+1; // se è dispari
}
//parent of the node idx
int parentIdx(int idx){
  return (int)(idx-1)/2;
}
// idx of 1st node of a level i
int firstIdx(int level){
  return (1 << level)- 1; 
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
                        
    // controlli sui buffer
    if (!memory){
      printf("\nErrore Creazione Buddy Allocator: Puntatore alla memoria fornita NULL\n");
      return -1;
    } 
    if (!bitmap_buffer){
      printf("\nErrore Creazione Buddy Allocator: Puntatore al buffer per la bitmap NULL\n");
      return -1;
    }       

    // controlli sulle dimensioni
    if (memory_size<= 0){
      printf("\nErrore Creazione Buddy Allocator: Dimensione della memoria deve essere > 0\n");
      return -1;
    } 
    if (bitmap_buffer_size<= 0){
      printf("\nErrore Creazione Buddy Allocator: Dimensione del buffer per la bitmap deve essere > 0\n");
      return -1;
    }              

    // controllo sul numero di livelli
    if (num_levels>=MAX_LEVELS){
      printf("\nErrore Creazione Buddy Allocator: numero di livelli superiore al massimo (20)\n");
      return -1;
    }

    // controllo sulla consistenza di min_bucket_size
    // con la dimensione della memoria ed il numero di livelli
    if (min_bucket_size != memory_size>>num_levels){
      printf("\nErrore Creazione Buddy Allocator: Invalid min_bucket_size\n");
      return -1;
    }

    // se la memoria non è una potenza di 2, verrà utilizzata la porzione di memoria
    // più grande possibile (disponibile) che è potenza di 2
    if (log2(memory_size) != floor(log2(memory_size))){
        memory_size = min_bucket_size << num_levels;
    }

    alloc->num_levels=num_levels;
    alloc->memory=memory;
    alloc->memory_size=memory_size;
    alloc->min_bucket_size=min_bucket_size;
    
    //controllo sulla dimensione della bitmap e memoria disponibile per allocarla
    int num_bits =(1 << (num_levels+1)) - 1; // il numero di bit sarà  (2 * 2^num_levels ) - 1 perchè si conta sempre il livello 0
                                            // es. num_levels = 3: L3: 8 bit, L2: 4bit, L1: 2bit, L0: 1bit
    if (bitmap_buffer_size < BitMap_getBytes(num_bits)){
      printf("\nErrore Creazione Buddy Allocator: memoria fornita insufficiente per contenere la Bitmap: necessari %d byes\n",BitMap_getBytes(num_bits));
      return -1;
    }
    // inizializzazione
    BitMap_init(&(alloc->bitmap),num_bits,bitmap_buffer);
    printf("Buddy Allocator Creato\nLivelli: %d\nDimensione della Memoria: %d\nNumero di bit nella bitmap: %d\nDimensione della bitmap: %d\nDimensione Bucket Minima: %d\n", num_levels,memory_size,num_bits,BitMap_getBytes(num_bits),min_bucket_size);
    return 0;
}

// cerca un buddy libero da restituire alla malloc, inserendo anche indice del blocco nella bitmap
// e dimensione nel blocco da restituire (per il funzionamento)
void* BuddyAllocator_getBuddy(BuddyAllocator* alloc, int level, int size){
  int bitmap_idx = -1;
  if (level == 0){ // radice
    int bit = BitMap_bit(&alloc->bitmap, firstIdx(level));
    if (bit == 0) bitmap_idx = 0;
    // se non è libero, bitmap_idx rimane a -1, così si indica che non ci sono blocchi liberi
  }
  else{
    int i = firstIdx(level); // itero su tutto il livello in cui devo cercare un buddy
    while (i< firstIdx(level+1)){ // fino all'inizio del livello successivo
      int bit = BitMap_bit(&alloc->bitmap, i); 
      if (bit == 0){
         bitmap_idx = i; 
         break;
      }
      i++;
    }
  }
  if (bitmap_idx == -1){ // se non ho trovato blocchi liberi
    return NULL;
  }
  else{ // ho trovato un blocco
    // si aggiorna la bitmap settando ad 1 i blocchi antenati e figli del blocco appena preso
    update_child(&alloc->bitmap,bitmap_idx,1); // entrambe le funzioni settano il bit che indica l'indice
    update_parent(&alloc->bitmap,bitmap_idx,1); // del blocco preso a 1 (essendo ricorsive): non c'è bisogno di farlo qui

    int block_size = alloc->min_bucket_size << (alloc->num_levels - level); // block_size = bucket_size * 2 ^num_level - level
                                                                            // perchè i livelli si contano dall'alto verso il basso, la dimensione
                                                                            // è al contrario

    char *ret = alloc->memory + ((startIdx(bitmap_idx)+1) * block_size); // l'indirizzo da restituire è calcolato sommando all'inizio
                                                                        // della memoria l'offset dell'indice nel suo livello * dimensione del blocco
    
    // ci salviamo nel blocco l'indice corrispettivo della bitmap
    //essendoci fatto spazio precedentemente
    ((int*)ret)[0] = bitmap_idx;
    ((int*)ret)[1] = size; // salvo anche la dimensione per controllare se deallocare il blocco con munmap o la free del buddy allocator
    return (void *)(ret + 2*sizeof(int)); // + size dell'indirizzo del blocco in bitmap e dimensione del blocco (originale)
  }
}

void* BuddyAllocator_malloc(BuddyAllocator* alloc, int size){

  // controlli su size
  if (size<0){
    printf("\nErrore di malloc: Invalid Size (<0)\n");
    return NULL;
  }
  if (size == 0) {
    printf("\nErrore di malloc: Impossibile allocare 0 byte\n");
    return NULL;
  }

  // aggiungo spazio per salvare l'indirizzo del blocco nella bitmap e la sua dimensione originale
  // per controllare nella pseudo_free se deallocare con buddy_free o munmap
  int org_size = size; // salvo la dimensione originale
  size += 2*sizeof(int); // overhead (8bytes)

  // controllo sullo spazio disponibile
  if (size > alloc->memory_size){
    printf("\nErrore di malloc: Memoria richiesta più grande della memoria totale disponibile\n");
    return NULL;
  }

  // we determine the level of the page
  int mem_size=(1<<alloc->num_levels)*alloc->min_bucket_size;
  // log2(mem_size): n bits to represent the whole memory
  // log2(size): n nits to represent the requested chunk
  // bits_mem_size-bits_size = depth of the chunk = level
  int  level=floor(log2(mem_size/size));
  // if the level is too small, we pad it to max
  if (level>alloc->num_levels){ level=alloc->num_levels;}

  printf("\nRichiesti: %d bytes (+ 8 bytes di overhead) , necessari %d bytes , al livello %d \n",org_size, alloc->min_bucket_size<<(alloc->num_levels - level), level);

  // cerco un blocco libero nella bitmap
  void* indirizzo = BuddyAllocator_getBuddy(alloc, level ,org_size);
  if (indirizzo == NULL){
    printf("Errore di malloc: nessun blocco di memoria libero disponibile\n");
  }
  else{
    printf("Allocazione riuscita: indirizzo %p\n", indirizzo);
    return indirizzo;
  }
  return NULL;
}

void BuddyAllocator_releaseBuddy(BuddyAllocator* alloc, int bit, void* mem){
  // controllo sulla double free
  if (BitMap_bit(&alloc->bitmap, bit) == 0){
     printf("\nErrore di deallocazione: Blocco di memoria all'indice: %p, già deallocato (double free). \n", mem);
    return;
  }
  // si aggiorna il bit dei figli a 0 ricorsivamente
  update_child(&alloc->bitmap,bit,0);
  // si aggiorna il bit del padre a 0 e si prova a fare la merge, sempre ricorsivamente
  merge(&alloc->bitmap,bit);
  printf("\nDeallocazione riuscita: Blocco di memoria all'indice %p libero \n", mem);
}

void BuddyAllocator_free(BuddyAllocator* alloc, void* mem){
  if (!mem){
    printf("\nErrore di deallocazione: Memoria da liberare vuota; Puntatore fornito NULL\n");
    return;
  }
  // ritroviamo il bit del buddy nel sistema, avendolo salvato in memoria
  int* p = (int*) mem;
  p--; // per saltare la size che era stata salvata nel blocco
  int idx = *(--p); // indice della bitmap del blocco
  BuddyAllocator_releaseBuddy(alloc,idx,mem);
}

// quando si dealloca un blocco si verifica se il suo buddy è libero, ed in caso si
// fa il merge, ovvero si libera anche il blocco padre dei buddy.
void merge(BitMap *bitmap, int bit){
    if (bit == 0) return; //radice
  
    int value = BitMap_bit(bitmap, bit);
    // sanity check
    if (value == 1){ 
      printf("\n Errore Fatale nella bitmap (merge su bit 1)\n");
      return;
    }
    // si trova l'indice del buddy e si vede sae è libero o no
    int buddy = buddyIdx(bit);
    value = BitMap_bit(bitmap,buddy);
    if (value == 1){ // se non è libero non si fa nulla
      return;
    }
    else { // altrimenti si setta il bit a 0 del padre facendo il "merge" dei figli, tutto ricorsivamente
      int parent = parentIdx(bit);
      BitMap_setBit(bitmap,parent,0);
      merge(bitmap,parent); // ricorsione in risalita
    }
}

// setta il bit stesso e il suo bit padre nella bitmap al valore passato come argomento (1 o 0)
void update_parent(BitMap *bitmap, int bit, int value) {
  BitMap_setBit(bitmap,  bit,value);
  if ( bit > 0) {
    update_parent(bitmap, parentIdx(bit), value); //ricorsione in salita dell'albero
  }
}

void update_child(BitMap *bitmap, int  bit, int value) {
  if ( bit < bitmap->num_bits) { 
    BitMap_setBit(bitmap,  bit, value);
    // si fa ricorsione binaria sui figli in discesa
    update_child(bitmap,  bit * 2 +1, value);  // sinistro
    update_child(bitmap,  bit * 2 +2, value);  // destro
  }
}

