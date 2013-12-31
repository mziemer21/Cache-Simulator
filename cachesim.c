/******************************************************************************
 * FILENAME: cahcesim.c
 * AUTHOR:   crf@cs.wisc.edu <Chris Feilbach>
 * DATE:     25 Oct 2013
 * PROVIDES: A functional, trace-driven, cache simulator for a unified, 
 *           write-back, write-allocate, single cache hierarchy.
 * MODIFIED BY:  Matthew Ziemer, #1, Michael Gilsdorf, #2
 *****************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>

#include "cache.h"
#include "trace_file_parser.h"

int main(int argc, char** argv)
{
	// Check argc, otherwise print usage statement.
	int retval = 0;
	if (argc != 5)
	{
		// Print usage statement.
		printf("\n\nusage: cachesim trace_path cache_size cache_associativity cache_block_size\n\n");
		return -1;
	} else {
		// Correct number of arguments were passed in.  
		// Determine whether the inputs actually describe a valid cache.
		
		// Convert inputs from ASCII strings to ints.
		int cache_size = atoi(argv[2]);
		int cache_associativity = atoi(argv[3]);
		int cache_block_size = atoi(argv[4]);
		
		// Check to make sure all inputs are greater than 0.
		if (cache_size <= 0)
		{
			retval = -1;
			printf("ERROR: Cache size (%d) must be greater than 0.\n", cache_size);
		}
		if (cache_associativity <= 0)
		{
			retval = -1;
			printf("ERROR: Cache associativity (%d) must be greater than 0.\n",
				   cache_associativity);
		}
		if (cache_block_size <= 0)
		{
			retval = -1;
			printf("ERROR: Cache block size (%d) must be greater than 0.\n",
				   cache_block_size);
		}
		
		// Do some simple error validation: size must be bigger than block_size.
		if (cache_size < cache_block_size || cache_size < cache_associativity)
		{
			retval = -1;
			printf("ERROR: Cache size is less than cache block size (%d) or cache associativity (%d).\n",
				   cache_block_size, cache_associativity);
		}
		
		// If retval is -1 bad input was detected, just return now.
		if (retval == -1) return retval;
		
		// Validate that these cache parameters make sense for a 32-bit
		// machine.
		struct cache* c;
		c = create_cache_struct(cache_size,cache_associativity,cache_block_size);
		// Open the trace file.
		open_trace_file(argv[1]);

		while(is_next_reference())
		{
			struct memory_reference mr;
			mr = get_next_reference();

			//printf("Memory reference: %x\n", mr.address);
			do_reference(mr,c);
		}

		// Print statistics.
		int references = c->perf.instruction_reference_count + c->perf.data_read_reference_count + c->perf.data_write_reference_count;
		int misses = c->perf.instruction_miss_count + c->perf.data_read_miss_count + c->perf.data_write_miss_count;
		int hits = references - misses;
		printf("Total number of memory references is (%d)\n", references);
		printf("Total number of hits is (%d)\n", hits);
		printf("The hit ratio is (%f)\n", (double) hits/references);
		// Free the cache structure.
		free_cache_struct(c);
	}
	return retval;
}
