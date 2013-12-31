/******************************************************************************
 * FILENAME: trace_file_parser.c
 * AUTHOR:   crf@cs.wisc.edu <Chris Feilbach>
 * DATE:     03 Nov 2013
 * PROVIDES: A C module that is responsible for opening a file of address traces
 *           and returning them one by one to the cache simulator.
 *****************************************************************************/

#include "trace_file_parser.h"

static FILE* trace_file;

/**********************************************************
 * bool open_trace_file(const char* path)
 * Opens a trace file.  If the file is not able to be
 * opened an error message is printed to the screen.
 *
 * ARGUMENTS:
 * x path - Path of the trace file.
 *
 * RETURN VALUE:
 * x Returns TRUE on successfully opening the trace, FALSE
 *   otherwise.
 *********************************************************/
bool open_trace_file(const char* path)
{
	trace_file = fopen(path, "r");
	
	// Check to see whether the open call failed.
	if (trace_file == NULL)
	{
		printf("ERROR: Unable to open trace file (%s)\n", path);
		return false;
	}
	
	// Trace file opened.
	return true;
}

/**********************************************************
 * bool is_next_reference(void)
 * Determines if another reference exists in the trace
 * file.
 *
 * RETURN VALUE:
 * x Returns TRUE if another memory reference exists,
 *   FALSE otherwise.
 *********************************************************/
bool is_next_reference(void)
{
	return !feof(trace_file);
}

/**********************************************************
 * struct memory_reference get_next_reference(void)
 * Gets the next memory reference from the trace file of
 * the form:
 *
 * hexidecimal_address reference_type
 *
 * and converts it into a memory_reference structure.
 *
 * RETURN VALUE:
 * x A memory_reference struct containing the address of 
 *   the memory reference and type, if one exists.  If 
 *   there are no more memory references in the trace
 *   file an error messsage will be printed to stdout and
 *   the values in the struct that is returned are 
 *   undefined.
 *********************************************************/
struct memory_reference get_next_reference(void)
{
	struct memory_reference retval;
	
	// First make sure a reference exists.
	if (!is_next_reference())
	{
		printf("ERROR: Attempted to read reference from trace file when no more references exist.\n");
		return retval;
	}
	
	// Use sscanf to make life easy.
	char reference_type;
	if(fscanf(trace_file, "%x %c\n", &retval.address, &reference_type) != 2)
	{
		// sscanf() did not read the correct number of tokens.
		printf("ERROR: sscanf() was unable to parse trace file.\n,");
		return retval;
	}
	
	// The reference_type must be I, R, or W.
	switch (reference_type)
	{
		case 'I':
			// This was an instruction fetch.
			retval.type = INSTRUCTION;
			break;
		case 'R':
			// This was a data read.
			retval.type = DATA_READ;
			break;
		case 'W':
			// This was a data write.
			retval.type = DATA_WRITE;
			break;
		default:
			// Unknown type.
			printf("ERROR: Unknown reference type detected (%c).\n", 
				   reference_type);
			break;
	}
	
	// Return the memory_reference struct.
	return retval;
}
