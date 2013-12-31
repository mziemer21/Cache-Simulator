/******************************************************************************
 * FILENAME: trace_file_parser.h
 * AUTHOR:   crf@cs.wisc.edu <Chris Feilbach>
 * DATE:     30 Oct 2013
 * PROVIDES: Header file for trace_file_parser.h.
 *****************************************************************************/
 
#ifndef TRACE_FILE_PARSER_H
#define TRACE_FILE_PARSER_H

#include <stdio.h> // printf(), fopen(), fscanf()
#include <stdbool.h> // true and false typedefs.

// Enumeration for memory reference types.  A memory reference can be
// caused by the CPU fetching an instruction, or the CPU reading or
// writing a data operand.
enum REFERENCE_TYPE {INSTRUCTION, DATA_READ, DATA_WRITE};

struct memory_reference
{
	unsigned int address;
	enum REFERENCE_TYPE type;
};

bool open_trace_file(const char* path);
bool is_next_reference(void);
struct memory_reference get_next_reference(void);

#endif

