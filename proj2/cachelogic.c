#include "tips.h"

/* The following two functions are defined in util.c */

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w); 

/* return random int from 0..x-1 */
int randomint( int x );

/*
  This function allows the lfu information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lfu information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);

  return buffer;
}

/*
  This function allows the lru information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lru information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);

  return buffer;
}

/*
  This function initializes the lfu information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lfu(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].accessCount = 0;
}

/*
  This function initializes the lru information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lru(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].lru.value = 0;
}

/* This is a helper function that shifts a number over by the number of bits in a mask */
unsigned int getShifty(unsigned int bin, unsigned int mask){
	int i = 0;
	unsigned int lastBit= 0x1;
	while ( ((mask&lastBit) == 0) && (i < 32) ){
		bin = bin >> 1;
		mask = mask >> 1;
		i++;
	}
	return bin;
}

/* This is the helper function we are using to simplify our accessMemory function
  it returns true if hit, false if miss, and updates the pointers
  to the correct values needed for either updating(case of hit), or replacing(case of miss) cache
*/
unsigned int computeLocation(unsigned int* compIndex, unsigned int* compTag, unsigned int* compOffset, unsigned int* compBlock, unsigned int addr){

  // compute the offset
  // uint_log2();
  *compOffset = (addr & (block_size-1));
  printf("compOffset = %i\n", *compOffset);
  
  unsigned int offset = uint_log2(block_size);
  *compIndex = ((addr >> offset) & (set_count-1));
  printf("compindex = %i\n", *compIndex);

  // compute the tag
  unsigned int tagMask = ( 0xffff-((block_size-1) + ((set_count-1)<<(block_size/2)) ) );

  *compTag = getShifty(addr, tagMask);
  printf("compTag = %i\n", *compTag);

  // loop through blocks (if associative) until we find the compBlock
  for (int i = 0; i < assoc; i++){
    if (cache[*compIndex].block[i].tag == *compTag){
      *compBlock = i;
      return 1;
    }
  }

  // We have a miss, we need to select the least recently used shifts {RANDOM, LRU, LFU} 
  if(policy == RANDOM){ // random
    *compBlock = (unsigned int)(((unsigned long)(&tagMask))%assoc);
  }
  else if(policy == LRU){ // LRU
    unsigned int lruIndex = 0;
    unsigned int lruMin = assoc-1;
    for (int i = 0; i < assoc; i++){
      if (cache[*compIndex].block[i].lru.value < lruMin){
        lruMin = cache[*compIndex].block[i].lru.value;
        lruIndex = i;
      }
    }
    *compBlock = lruIndex;
  }
  else{ // LFU
    printf("LFU not supported\n");
  }
  return 0;
}

void updateLRU(unsigned int compIndex, unsigned int compBlock){


  int newCache = 1;
  for (int i = 0; i < assoc; i++){
    if (cache[compIndex].block[i].lru.value != 0){
      newCache = 0;
      break;
    }
  }
  if(newCache==1){
    for(int i = 0; i < assoc; i++){
      cache[compIndex].block[i].lru.value = i;
    }
  }
  int cur = cache[compIndex].block[compBlock].lru.value;
  for(int i = 0; i < assoc; i++){
    if (cache[compIndex].block[i].lru.value > cur){
      cache[compIndex].block[i].lru.value--;
    }
  }
  cache[compIndex].block[compBlock].lru.value = assoc-1;
}

int getEnumVal(){
  // BYTE_SIZE = 0, HALF_WORD_SIZE, WORD_SIZE, DOUBLEWORD_SIZE, QUADWORD_SIZE, OCTWORD_SIZE
  switch(block_size){
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    case 8:
      return 3;
    case 16:
      return 4;
    case 32:
      return 5;
  }
  return -1;
}

/*
  This is the primary function you are filling out,
  You are free to add helper functions if you need them

  @param addr 32-bit byte address
  @param data a pointer to a SINGLE word (32-bits of data)
  @param we   if we == READ, then data used to return
              information back to CPU

              if we == WRITE, then data used to
              update Cache/DRAM
*/
void accessMemory(address addr, word* data, WriteEnable we)
{
  /* Declare variables here */
  unsigned int compIndex = 0;
  unsigned int compTag = 0;
  unsigned int compOffset = 0;
  unsigned int compBlock = 0;

  /* handle the case of no cache at all - leave this in */
  if(assoc == 0) {
    accessDRAM(addr, (byte*)data, WORD_SIZE, we);
    return;
  }
  __uint8_t transferUnit = getEnumVal();
  // If we are writing data
  if(we){
    // if we have a hit
    if(computeLocation(&compIndex, &compTag, &compOffset, &compBlock, addr)){
      printf("Index:::::: %u\n", compIndex);
      // update cache
      cache[compIndex].block[compBlock].data[compOffset] = *data;
      highlight_offset(compIndex, compBlock, compOffset, HIT);
      if(policy==LRU){
        updateLRU(compIndex, compBlock);
      }
      if(memory_sync_policy == WRITE_THROUGH){
        // update dram
      accessDRAM(addr, cache[compIndex].block[compBlock].data, transferUnit, we);
        cache[compIndex].block[compBlock].dirty = VIRGIN;
      }
      else{
        cache[compIndex].block[compBlock].dirty = DIRTY;
      }
    }
    // if we have a miss
    else{
      printf("Index:::::: %u\n", compIndex);
      if(memory_sync_policy == WRITE_BACK){
        // if dirty bit is dirty
        if(cache[compIndex].block[compBlock].dirty == DIRTY){
          // update dram
      accessDRAM(addr, cache[compIndex].block[compBlock].data, transferUnit, we);
        }
      }
      // update cache
      highlight_offset(compIndex, compBlock, compOffset, MISS);
      highlight_block(compIndex, compBlock);
      cache[compIndex].block[compBlock].data[compOffset] = *data;
      cache[compIndex].block[compBlock].dirty = VIRGIN;
      cache[compIndex].block[compBlock].tag = compTag;
      if(policy==LRU){
        updateLRU(compIndex, compBlock);
      }
      if(memory_sync_policy == WRITE_THROUGH){
        //update dram
      accessDRAM(addr, cache[compIndex].block[compBlock].data, transferUnit, we);
      }
    }
  }

  // If we are reading data
  else{
    // If we have a hit
    if(computeLocation(&compIndex, &compTag, &compOffset, &compBlock, addr)){
      printf("Index:::::: %u\n", compIndex);
      highlight_offset(compIndex, compBlock, compOffset, HIT);
      memcpy(data, cache[compIndex].block[compBlock].data+compOffset, 4);
      if(policy == LRU){
        updateLRU(compIndex, compBlock);
      }
    }
    // If we have a miss
    else{
      printf("Index:::::: %u\n", compIndex);
      highlight_offset(compIndex, compBlock, compOffset, MISS);
      highlight_block(compIndex, compBlock);
      // IS CONFUSION
      accessDRAM(addr, cache[compIndex].block[compBlock].data, transferUnit, we);
      memcpy(data, cache[compIndex].block[compBlock].data+compOffset, 4);
      cache[compIndex].block[compBlock].tag = compTag;
      if(policy == LRU){
        updateLRU(compIndex, compBlock);
      }
    }
  }

  // printf("set_count: %i\nassoc: %i\nblock_size: %i\npolicy: %i\nmemory_sync_policy: %i\n", set_count, assoc, block_size, policy, memory_sync_policy);

  /*
  You need to read/write between memory (via the accessDRAM() function) and
  the cache (via the cache[] global structure defined in tips.h)

  Remember to read tips.h for all the global variables that tell you the
  cache parameters

  The same code should handle random, LFU, and LRU policies. Test the policy
  variable (see tips.h) to decide which policy to execute. The LRU policy
  should be written such that no two blocks (when their valid bit is VALID)
  will ever be a candidate for replacement. In the case of a tie in the
  least number of accesses for LFU, you use the LRU information to determine
  which block to replace.

  Your cache should be able to support write-through mode (any writes to
  the cache get immediately copied to main memory also) and write-back mode
  (and writes to the cache only gets copied to main memory when the block
  is kicked out of the cache.

  Also, cache should do allocate-on-write. This means, a write operation
  will bring in an entire block if the block is not already in the cache.

  To properly work with the GUI, the code needs to tell the GUI code
  when to redraw and when to flash things. Descriptions of the animation
  functions can be found in tips.h
  */

  /* Start adding code here */


  /* This call to accessDRAM occurs when you modify any of the
     cache parameters. It is provided as a stop gap solution.
     At some point, ONCE YOU HAVE MORE OF YOUR CACHELOGIC IN PLACE,
     THIS LINE SHOULD BE REMOVED.
  */
  //accessDRAM(addr, (byte*)data, WORD_SIZE, we);
}
