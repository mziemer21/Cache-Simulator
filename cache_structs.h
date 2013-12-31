/******************************************************************************
 * FILENAME: cache_structs.h
 * AUTHOR:   crf@cs.wisc.edu <Chris Feilbach>
 * DATE:     25 Oct 2013
 * PROVIDES: Provides structs to model cache memories.  The cache struct
 *           contains information about the cache being simulated.  The 
 *           address_info struct contains information about the address width
 *           of the machine, the and the number of offset,index, and tag bits
 *           used by the cache.  The cache_block structure contains information 
 *           about a cache block.  The cache_perf structure contains information 
 *           about hit rate for the cache being simulated.
 *****************************************************************************/
 
#ifndef CACHE_STRUCTS_H
#define CACHE_STRUCTS_H

#include <stdbool.h> // #defines for true, false, and bool typedef.

// The address_info structure contains the width of the address,
// and the widths of the offset, index, and tag fields for the
// cache.
struct address_info
{
	unsigned int address_width;
	unsigned int offset_width;
	unsigned int index_width;
	unsigned int tag_width;
};

// The cache_perf structure contains the necessary attributes to determine
// hit rates for instruction fetches, data reads, and data writes.
struct cache_perf
{
	unsigned int instruction_reference_count;
	unsigned int instruction_miss_count;
	unsigned int data_read_reference_count;
	unsigned int data_read_miss_count;
	unsigned int data_write_reference_count;
	unsigned int data_write_miss_count;
};

// The cache struct contains information about the cache, a pointer to 
// the blocks in the cache, and a cache_perf structure to record cache
// performance.
struct cache
{
	unsigned int size;
	unsigned int associativity;
	unsigned int number_of_sets;
	unsigned int block_size;
	
	struct address_info addr_info;
	
	struct cache_block** blocks;
	struct cache_perf perf;
};


// The cache_block structure represents a cache block.  It contains status
// bits found in each cache block (valid and dirty), along with the tag
// for that block and data used to implement the LRU replacement policy.  
// Normally data would be found in this structure as well, but this simulator
// doesn't need data to determine the hit rate ... all you need is the
// addresses!
struct cache_block
{
	bool valid;
	bool dirty;
	unsigned int lru_value;
	unsigned int tag;
};


#endif
