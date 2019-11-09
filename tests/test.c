#include <stdio.h>

unsigned int block_size;
unsigned int set_count;
unsigned int assoc;
unsigned int policy; //(RANDOM 0), (LRU 1)

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

unsigned int computeLocation(unsigned int* compIndex, unsigned int* compTag, unsigned int* compOffset, unsigned int* compBlock, unsigned int addr){

  // compute the offset
  *compOffset = (addr & (block_size-1));
  printf("compOffset = %i\n", *compOffset);
  
  // compute index
  *compIndex = ((addr >> (block_size/2)) & (set_count-1));
  printf("compindex = %i\n", *compIndex);

  // compute the tag
  unsigned int tagMask = ( 0xffff-((block_size-1) + ((set_count-1)<<(block_size/2)) ) );

  *compTag = getShifty(addr, tagMask);
  printf("compTag = %i\n", *compTag);

  // loop through blocks (if associative) until we find the compBlock

  //if hit, return 1

  //if miss, return 0 
}

int main(){

	unsigned int addr = 0;
	unsigned int compIndex = 0; 
	unsigned int compTag = 0; 
	unsigned int compOffset = 0; 
	unsigned int compBlock = 0;

	/*
	// Ex1 
	unsigned int ar[] = { 3, 180, 43, 2, 191, 88, 190, 14, 181, 44, 186, 253 };
	block_size = 2;
	set_count = 4;
	assoc = 3;
	policy = 1;
	*/

	/*
	// Ex 2
	unsigned int ar[] = { 0x27, 0x0f, 0x0c, 0x11, 0x32, 0x25, 0x0e, 0x21, 0x31 };
	block_size = 4;
	set_count = 2;
	assoc = 2;
	policy = 1;
	*/

	/*
	// Ex 3
	unsigned int ar[] = { 3, 180, 43, 2, 191, 88, 190, 14, 181, 44, 186, 253 };
	block_size = 1;
	set_count = 16;
	assoc = 1;
	policy = 1;
	*/

	
	// Ex 4
	unsigned int ar[] = { 3, 180, 43, 2, 191, 88, 190, 14, 181, 44, 186, 253 };
	block_size = 2;
	set_count = 8;
	assoc = 1;
	policy = 1;
	

	for (int i = 0; i < 12; i++){

		addr = ar[i];
		printf("addr: %i\n", addr);

		computeLocation(&compIndex, &compTag, &compOffset, &compBlock, addr);
	}

	return 0;

}

















