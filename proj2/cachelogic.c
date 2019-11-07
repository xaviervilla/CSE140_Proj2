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

/* This is the helper function we are using to simplify our accessMemory function
  it returns true if hit, false if miss, and updates the pointers
  to the correct values needed for either updating(case of hit), or replacing(case of miss) cache
*/
unsigned int computeLocation(unsigned int* compIndex, unsigned int* compTag, unsigned int* compOffset, unsigned int* compBlock, address addr){
  
  // compute index
  compIndex = ((addr >> block_size) && (set_count-1)) -1;
  printf("compindex = %i\n", compIndex);

  // compute the offset
  compTag = (addr && (block_size-1)) -1;

  printf("compTag = %i\n", compTag);
  // compute the tag

  // loop through blocks (if associative) until we find the compBlock

  //if hit, return 1

  //if miss, return 0
  
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
  unsigned int compIndex = 1;
  unsigned int compTag = 1;
  unsigned int compOffset = 1;
  unsigned int compBlock = 1;

  /* handle the case of no cache at all - leave this in */
  if(assoc == 0) {
    accessDRAM(addr, (byte*)data, WORD_SIZE, we);
    return;
  }

  // If we are writing data
  if(we){
    // if we have a hit
    if(computeLocation(&compIndex, &compTag, &compOffset, &compBlock, addr)){
      // update cache
      cache[compIndex].block[compBlock].data[compOffset] = *data;
      if(memory_sync_policy == WRITE_THROUGH){
        // update dram
        accessDRAM(addr, (byte*)data, WORD_SIZE, we);
        cache[compIndex].block[compBlock].dirty = VIRGIN;
      }
      else{
        cache[compIndex].block[compBlock].dirty = DIRTY;
      }
    }
    // if we have a miss
    else{
      if(memory_sync_policy == WRITE_BACK){
        // if dirty bit is dirty
        if(cache[compIndex].block[compBlock].dirty == DIRTY){
          // update dram
          accessDRAM(addr, (byte*)data, WORD_SIZE, we);
        }
      }
      // update cache
      cache[compIndex].block[compBlock].data[compOffset] = *data;
      cache[compIndex].block[compBlock].dirty = VIRGIN;
      if(memory_sync_policy == WRITE_THROUGH){
        //update dram
        accessDRAM(addr, (byte*)data, WORD_SIZE, we);
      }
    }
  }

  // If we are reading data
  else{
    computeLocation(&compIndex, &compTag, &compOffset, &compBlock, addr);
    printf("Read not yet enabled\n");
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
  accessDRAM(addr, (byte*)data, WORD_SIZE, we);
}
