Cache-Simulator
===============
cachesim.c simulates a cache's functionality 
Args: traceFile, cacheSize, setAssociativity, blockSize
Arg specifications: cacheSize must be a power of 2 (8192 = 16kb, ), set associativity in this example is always one (a direct 
mapped cache).

strgen_trace are the memory accesses generated when strgen (https://github.com/mziemer21/String-Int-Conversions) was executed. File gcc_trace are the 
memory accesses generated when strgen.c was compiled. 

cache.c, cache.h constructs a cache and simulated memory operations on the cache using a trace file opened by trace_file_parser.c

cache_structs.h contains structs  that are used my cache.c

trace_file_parser.c, trace_file_parser.h opens and reads a trace file for use in cache.c

All files can be compiled using the Makefile (compiled using: gcc -Wall -m32 -O)