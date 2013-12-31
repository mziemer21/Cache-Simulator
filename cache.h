/******************************************************************************
 * FILENAME: cache.h
 * AUTHOR:   crf@cs.wisc.edu <Chris Feilbach>
 * DATE:     25 Oct 2013
 * PROVIDES: Header file for cache.c.
 *****************************************************************************/
 
#ifndef CACHE_H
#define CACHE_H

#include <stdbool.h>

#include "cache_structs.h"
#include "trace_file_parser.h"

// Forward declarations.
struct cache* create_cache_struct(const unsigned int size,
				  const unsigned int associativity,
				  const unsigned int block_size);
void free_cache_struct(struct cache* c);
void do_reference(const struct memory_reference m, struct cache* c);

#endif
