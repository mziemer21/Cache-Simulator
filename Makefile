#* MODIFIED BY:  Matthew Ziemer, #1, Michael Gilsdorf, #2

CC     = gcc
# argument CFLAGS is missing.  Place it here.
CFLAGS = -Wall -m32 -O

# fill in what is needed for target 'all'
all: 			cachesim.o cache.o trace_file_parser.o
			$(CC) -o cachesim cachesim.o cache.o trace_file_parser.o $(CFLAGS)

cachesim:		cachesim.c
			$(CC) -c -o cachesim.o cachesim.c $(CFLAGS)

cache:			cache.c
			$(CC) -c -o cache.o cache.c $(CFLAGS)

trace_file_parser:	trace_file_parser.c
			$(CC) -c -o trace_file_parser.o trace_file_parser.c $(CFLAGS)

