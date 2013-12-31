/******************************************************************************
 * FILENAME: cache.c
 * AUTHOR:   crf@cs.wisc.edu <Chris Feilbach>
 * DATE:     28 Oct 2013
 * PROVIDES: A file to containing code validate cache parameters and simulate 
 *           cache memories.
 * MODIFIED BY:  Matthew Ziemer, #1, Michael Gilsdorf, #2
 *****************************************************************************/

#include <stdio.h> // printf()
#include <stdlib.h> // malloc()
#include "cache.h"

// Static forward declarations.
static void determine_address_widths(struct cache* c);

static bool is_miss(const unsigned int tag, const unsigned int index,
                    const struct cache* c);
static void handle_miss(const unsigned int tag, const unsigned int index,
                        const bool write, const struct cache* c);
static void update_lru_info(const unsigned int tag, const unsigned int index,
                            const struct cache* c);

static unsigned int get_index_from_address(const unsigned int address,
                                           const struct cache* c);
static unsigned int get_tag_from_address(const unsigned int address,
                                         const struct cache* c);
static void determine_address_widths(struct cache* c);

static int log_two(const unsigned int n);
static bool is_power_of_two(const unsigned int n);

/**********************************************************
 * struct cache* create_cache_struct(const unsigned int size,
 *				     const unsigned int associativity,
 *				     const unsigned int block_size)
 *
 * This function takes in a cache size, associativity, and 
 * block size and determines whether these parameters make 
 * up a valid cache memory.
 *
 * For a cache memory to be valid the block_size must be a
 * power of two, along with the number of cache sets.  There
 * are no constraints placed on associativity and size, so long
 * as the power of two constraints are met.  This allows for
 * more advanced cache memories to be simulated if desired.
 *
 * Upon confirming that the cache parameters are valid, the 
 * function creates a cache structure and returns a pointer
 * to the structure.  If the parameters are invalid, the 
 * pointer is NULL.
 *
 * ARGUMENTS:
 * x size - Size of the cache memory.
 * x associativity - Associativity of the cache memory.
 * x block_size - Block size of the cache memory.
 *
 * RETURN VALUE:
 * x Returns a pointer to a cache structure if arguments
 *   specified represent a valid cache memory.  Returns
 *   NULL otherwise.
 *********************************************************/
struct cache* create_cache_struct(const unsigned int size,
				  const unsigned int associativity,
				  const unsigned int block_size)
{
	// If block_size is not a power of two this cache memory
	// cannot be simulated.
	if (!is_power_of_two(block_size))
	{
		printf("ERROR: Cache block size (%d) is not a power of two.\n",block_size);
		return NULL;
	}
	
	// Cache memories are organized as a collection of sets,
	// where each set can have multiple elements.  The number
	// of elements per set is the associativity of the cache.
	
	// When a cache is asked to fetch an instruction or read/write
	// data, it must first ask the question: "if this instruction
	// or data is in the cache, what set would it be in?".  The
	// index bits are used to select a set, and then the elements
	// are searched to see whether they contain the requested word.
	
	// In order use index bits in a cache memory, there must be 
	// a power of two number of sets.
	unsigned int number_of_sets = size / (associativity * block_size);
	
	// Check to see whether number_of_sets is a power of 2 and that
	// it is non-zero.
	if (number_of_sets == 0)
	{
		printf("ERROR: Number of sets (%d) must be non-zero.\n", number_of_sets);
		return NULL;
	}
	if (!is_power_of_two(number_of_sets))
	{
		printf("ERROR: Number of sets (%d) must be a power of two.\n",number_of_sets);
		return NULL;
	}
	
	// Allocate a new cache structure.
	struct cache* retval = malloc(sizeof(struct cache));
	// Check malloc to make sure it actually allocated space.
	if (retval == NULL)
	{
		printf("ERROR: Failed to allocate cache structure.\n");
		return NULL;
	}
	
	// Allocate an array of cache_block arrays.  The number of 
	// arrays needed is equal to the number of sets.
	retval->blocks = (struct cache_block**)malloc(number_of_sets*sizeof(struct cache_block*));
	if (retval->blocks == NULL)
	{
		printf("ERROR: Failed to allocate array for cache_block structures.\n");
		return NULL;
	}
	
	// Now allocate the arrays for each of the sets AND set up the 
	// information to implement the LRU (Least Recently Used) replacement
	// policy.
	unsigned int i = 0;
	for(i=0;i<number_of_sets;i++)
	{
		// Allocate the array of cache_block structures.
		// The number of elements in the array is equal to
		// the associativity.
		retval->blocks[i] = (struct cache_block*)malloc(associativity*sizeof(struct cache_block));
		unsigned int j = 0;
		// Initialize each cache block.
		for(j=0;j<associativity;j++)
		{
			retval->blocks[i][j].valid = false;
			retval->blocks[i][j].dirty = false;
			retval->blocks[i][j].tag = 0;
		}
	}

	// Store the size, associativity, block size in the 
	// cache structure.
	retval->size = size;
	retval->associativity = associativity;
	retval->block_size = block_size;
	retval->number_of_sets = number_of_sets;
	
	// Determine the width of the offset, index, and tag bits.
	determine_address_widths(retval);
	
	// Set cache performance stats to 0.
	retval->perf.instruction_reference_count = 0;
	retval->perf.instruction_miss_count = 0;
	retval->perf.data_read_reference_count = 0;
	retval->perf.data_read_miss_count = 0;
	retval->perf.data_write_reference_count = 0;
	retval->perf.data_write_miss_count = 0;
	
	// The cache structure is set up.
	return retval;
}

/**********************************************************
 * void free_cache_struct(struct cache* c)
 * Frees any dynamically allocated memory in the cache
 * structure c.
 * 
 * ARGUMENTS
 * x c - Cache structure to free.
 *
 * RETURN VALUE
 * x None.
 *********************************************************/
void free_cache_struct(struct cache* c)
{
	unsigned int i;
	for(i=0;i<c->number_of_sets;i++)
	{
		free(c->blocks[i]);
	}
	free(c->blocks);
	free(c);
}

/**********************************************************
 * static void determine_address_widths(struct cache* c)
 * Determines the widths of the offset, index, and tag 
 * fields.  These fields are determined using the width of
 * the machine, which is inferred by getting the size of a
 * void pointer.
 *
 * c will be updated with the appropriate values in it's
 * address_info structure on success.  On failure those
 * values will contain 0s.
 * ARGUMENTS:
 * x c is the cache structure.
 *********************************************************/
static void determine_address_widths(struct cache* c)
{
	// Determine the machine width, convert to bits.
	unsigned int width = sizeof(void*) << 3;
	
	// The width of the offset is log_two(cache_block_size).
	unsigned int offset_width = log_two(c->block_size);
	// The width of the index is log_two(number_of_sets).
	unsigned int index_width = log_two(c->number_of_sets);
	// The width of the tag is machine width - offset width - index_width.
	unsigned int tag_width = width - offset_width - index_width;
	
	// Assign 0s to address_width structure in c.
	c->addr_info.address_width = 0;
	c->addr_info.offset_width = 0;
	c->addr_info.index_width = 0;
	c->addr_info.tag_width = 0;
	
	// Do error checking.
	if (offset_width == -1)
	{
		printf("ERROR: Cache block size (%d) must be a power of two.\n",c->block_size);
	} else c->addr_info.offset_width = offset_width;
	if (index_width == -1)
	{
		printf("ERROR: Number of sets in cache (%d) must be a power of two.\n", c->number_of_sets);
	} else c->addr_info.index_width = index_width;
	
	// Assign the address width and the tag.
	if (offset_width != -1 && index_width != -1)
	{
		c->addr_info.address_width = width;
		c->addr_info.tag_width = tag_width;
	}
}

/**********************************************************
 * void do_reference(const struct memory_reference m,
 *		     struct cache* c)
 *
 * This function handles a memory reference.  It determines
 * if a miss occurs, handles it if it does, updates LRU
 * information, and updates various counters indicating 
 * the number of references and misses that occurred.
 *
 * ARGUMENTS:
 * x m - Struct containing information about the memory
 *       reference to simulate.
 * x c - Structure describing the cache memory being 
 *       simulated.
 *********************************************************/
void do_reference(const struct memory_reference m, struct cache* c)
{
	// First, determine if this memory reference causes a miss.
	// Get the tag and the index from the address.
	unsigned int tag = get_tag_from_address(m.address,c);
	
	// Determine the index for this memory reference.  This determines
	// the set where the requested word *could* be.
	unsigned int index = get_index_from_address(m.address,c);
	
	// Now that index and tag are known, determine if a miss will occur.
	// If a miss does happen, a different set of things need to occur
	// versus when a hit occurs.
	if (is_miss(tag,index,c))
	{
		// A miss occurred.  Handle it.
		// If this is a write, then set the dirty bit.
		bool is_write = false;
		if (m.type == DATA_WRITE) is_write = true;

		printf("Miss: %x\n", m.address);		
		handle_miss(tag,index,is_write,c);
		
		// handle_miss sets the tag, valid, and dirty bits.
		// The LRU occurs the same way regardless of whether a cache
		// hit or cache miss occurs.
		
		// Update counters.
		// Determine type, then update appropriate counter.
		switch (m.type)
		{
		case INSTRUCTION:
			c->perf.instruction_miss_count++;
			break;
		case DATA_READ:
			c->perf.data_read_miss_count++;
			break;
		case DATA_WRITE:
			c->perf.data_write_miss_count++;
			break;
		default:
			// Do nothing.
			break;
		}
		
	} else {
		// A hit occurred.  Nothing to do.
	}
	
	// Update LRU information.
	update_lru_info(tag,index,c);
	
	// Update counters.
		switch (m.type)
		{
		case INSTRUCTION:
			c->perf.instruction_reference_count++;
			break;
		case DATA_READ:
			c->perf.data_read_reference_count++;
			break;
		case DATA_WRITE:
			c->perf.data_write_reference_count++;
			break;
		default:
			// Do nothing.
			break;
		}	
}

/**********************************************************
 * static bool is_miss(const unsigned int tag,
 *		       const unsigned int index,
 * 		       const struct cache* c)
 * 
 * Determines for a given cache c, whether the tag tag and
 * index index cause a miss.  This function works for 
 * direct mapped, n-way set associative, and fully 
 * associative cache memories.
 *
 * ARGUMENTS:
 * x tag   - Tag of the address of the requested word.
 * x index - Index of the address of the requested word.
 * x c     - Cache memory being simulated.
 *
 * RETURN VALUE:
 * x Returns TRUE if a cache miss occurs, false otherwise.
 *********************************************************/
static bool is_miss(const unsigned int tag, const unsigned int index,
					const struct cache* c)
{
	// Loop over the set specified by the index, and valid bits and tags.
	struct cache_block* cb;
	
	// Get the array holding the elements for the set indexed by index.
	cb = c->blocks[index];
	
	unsigned int i = 0;
	bool miss = true;
	// Loop over the elements in the set.
	for (i=0;i<c->associativity;i++)
	{
		// Get the block.
		struct cache_block temp = c->blocks[index][i];
		
		// Check the tag AND the valid bit.  If tags are 
		// equal, and valid bit is set, then a miss did
		// not occur.
		if (temp.tag == tag && temp.valid)
		{
			miss = false;
			break;
		}
	}
	
	return miss;
}

/**********************************************************
 * static void handle_miss(const unsigned int tag,
 *			   const unsigned int index,
 *			   const bool write, 
 *			   const struct cache* c)
 *
 * This function handles when a miss occurs.  It must do
 * the following:
 * x Find the block to replace.
 * x Set the valid bit.
 * x This function does not need to update LRU information.
 *
 * ARGUMENTS:
 * x tag   - Tag of the requested word that caused the miss.
 * x index - The index of the requested word that caused the miss.
 * x c     - The struct describing the cache memory under siumulation.
 *********************************************************/
static void handle_miss(const unsigned int tag, const unsigned int index,
			const bool write, const struct cache* c)
{
	/*This function is specialized to work only for the case of a direct
 	*mapped cache. This is a cache where the associativity field within
	*the struct cache has the value 1. If the associativity is not 1
	*an error message is printed and the program exits.*/  
	if(c->associativity != 1)
	{
		printf("ERROR: This code only handles associativity of 1.\n");
		exit(1);
	}
	
	/*Sets valid bit to true, indicating that the data is good.*/	
	c->blocks[index][0].valid = true;
	/*Assignes dirty to the value that is passes into the function
 	*indicating if it needs to be written to memory.*/	
	c->blocks[index][0].dirty = write;
	/*Updates the tag bit to the new tag*/
	c->blocks[index][0].tag = tag;
}

/**********************************************************
 * static void update_lru_info(const unsigned int tag,
 *			       const unsigned int index,
 *			       const struct cache* c)
 *
 * This function updates the LRU information for a set.
 * It takes in the tag and index of the most recent memory
 * reference, which are used to find the specific block in 
 * the cache.  It then updates all the LRU values in that set 
 * to reflect the most recent memory reference.
 *
 * ARGUMENTS:
 * x tag   - Tag word of the address of the requested word.
 * x index - Index of the address of the requested word.
 * x c     - Struct describing the cache under simulation.
 *********************************************************/
static void update_lru_info(const unsigned int tag, const unsigned int index,
			    const struct cache* c)
{
	// Determine whether the LRU data even needs to be updated.
	// LRU information is stored as follows:
	// If the value stored in the LRU field is 0, it means that
	// the block was the most recently used.  The larger the number 
	// in the LRU field, the longer the time has been since that
	// block was referenced.
	
	// Determine where the block is in the set.  This
	// function assumes that a cache hit will occur.
	unsigned int i = 0;
	unsigned int block_index = 0;
	for(i=0;i<c->associativity;i++)
	{
		// Check if this block is valid, and whether the tags match.
		if (c->blocks[index][i].valid && c->blocks[index][i].tag == tag)
		{
			// Match!
			// If the LRU is 0, this means that the block is the 
			// most recently used, and no change has occurred with 
			// which block is the least recently used.
			if (!c->blocks[index][i].lru_value) return;
			
			// Otherwise, save the block index.
			block_index = i;
		}
	}
	
	// The block index was non-zero.  Increment every element in the set,
	// and set the element in block_index to 0, to denote this is the 
	// most recently used block.
	
	// By denoting most recently used and setting it to 0, it makes it 
	// very easy to find the least recently used block ... this is the
	// one with the largest value in the LRU field in the cache_block
	// struct.
	
	for(i=0;i<c->associativity;i++)
	{
		// Increment every lru field with the exception
		// of the one stored in block_index.
		if (i == block_index) c->blocks[index][i].lru_value = 0;
		if (i != block_index) c->blocks[index][i].lru_value++;
	}
	
}
 
 /*********************************************************
  * static unsigned int get_index_from_address(const unsigned int address,
  *					       const struct cache* c)
  * This function returns the index bits of the address for
  * a cache memory c.
  *
  * ARGUMENTS:
  * x address - Address to get index bits from.
  * x c       - Structure describing cache memory under
  *             simulation.
  ********************************************************/
static unsigned int get_index_from_address(const unsigned int address,
					   const struct cache* c)
{
	// Determine the number of sets, and subtract 1 to
	// turn it into a mask.
	unsigned int mask = (1 << c->addr_info.index_width) - 1;
	return (address >> c->addr_info.offset_width) & mask;
}
 
/**********************************************************
 * static unsigned int get_tag_from_address(const unsigned int address,
 *					    const struct cache* c)
 *
 * This function returns the tag bits of the address for a
 * cache memory c.
 *
 * ARGUMENTS:
 * x address - Address of the memory reference.
 * x c       - Structure describing cache memory under
 *             simulation.
 *
 * RETURN VALUE:
 * x Returns the tag of the address address.
 *********************************************************/
static unsigned int get_tag_from_address(const unsigned int address,
					 const struct cache* c)
{
	// The tag is the rightmost bits of the address.  If the
	// number of offset and index bits are calculated, then
	// a shift can be performed to get the tag.
	unsigned int bits_to_shift;
	bits_to_shift = c->addr_info.offset_width + c->addr_info.index_width;
	
	return address >> bits_to_shift;
}
 
/**********************************************************
 * static int log_two(const unsigned int n)
 * For a given number that is a power of two, log_two(n)
 * is performed.  Otherwise a value of -1 is returned,
 * indicating failure.
 *
 * ARGUMENTS:
 * x n - Input to log_two(n).
 *
 * RETURN VALUE:
 * Returns log_two(n) if n is a power of two, -1 otherwise.
 *********************************************************/
static int log_two(const unsigned int n)
{
	// Determine if this even if a power of 2 first.
	if (!is_power_of_two(n)) return -1;
	
	int power = 0;
	int i = 0;
	
	unsigned int number = n;
	
	// Find the first 1, then return it.
	for(i=0;i<sizeof(int)<<3;i++)
	{
		if ((number & 1) == 1)
		{
			power = i;
			return power;
		}
		number = number >> 1;
	}
	
	// n is not a power of two.
	return -1;
}
 
/**********************************************************
 * static bool is_power_of_two(const unsigned int n)
 * Determines if n is a power of two (ie 2^x = n, where x is
 * an integer). 
 * This can be determined by counting the number of binary 
 * bits that '1' in n.  If the answer is 1, then it is.  Any 
 * other value is false.
 * ARGUMENTS:
 * x n - Input.
 * RETURNS:
 * x TRUE if n is a power of two, FALSE otherwise.
 *********************************************************/
static bool is_power_of_two(const unsigned int n)
{
	unsigned int count = 0;
	unsigned int i = 0;
	unsigned int number = n;
	
	// Count the bits.
	for(i=0;i<sizeof(int)<<3;i++)
	{
		if (number & 1) count++;
		number = number >> 1;
	}
	
	return (count == 1);
}
 
