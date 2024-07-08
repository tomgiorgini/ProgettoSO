#include <assert.h>
#include "bit_map.h"

// returns the number of bytes to store bits booleans
int BitMap_getBytes(int bits){
  if (bits%8 == 0) return bits/8;
  else return bits/8 +1;
}

// initializes a bitmap on an external array
void BitMap_init(BitMap* bit_map, int num_bits, char* buffer){
  bit_map->buffer=buffer;
  bit_map->num_bits=num_bits;
  bit_map->buffer_size=BitMap_getBytes(num_bits);
}

// sets a the bit bit_num in the bitmap
// status= 0 or 1
void BitMap_setBit(BitMap* bit_map, int bit_num, int status){
  if (status != 0 && status != 1){
    printf("\nErrore Bitmap: Invalid Status\n");
  }
  int byte_num=bit_num>>3; // bytes = bits / 8
  assert(byte_num<bit_map->buffer_size);
  int bit_in_byte=bit_num&0x07; // i 3 bit meno significativi sono la posizione all'interno del byte
  if (status) {
    bit_map->buffer[byte_num] |= (1<<bit_in_byte);
  } else {
    bit_map->buffer[byte_num] &= ~(1<<bit_in_byte);
  }
}

// inspects the status of the bit bit_num
int BitMap_bit(const BitMap* bit_map, int bit_num){
  int byte_num=bit_num>>3; // bytes = bits / 8
  assert(byte_num<bit_map->buffer_size);
  int bit_in_byte=bit_num&0x07; // i 3 bit meno significativi sono la posizione all'interno del byte
  return (bit_map->buffer[byte_num] & (1<<bit_in_byte))!=0;
}