# Malloc-Implementation
Implementation of C malloc using segregated explicit free lists with an approximate best-fit search.

# Details on Malloc implementation
mm.c - malloc/free/realloc implemented using segregated explicit free lists
This library implements memory allocation operations by using an approximate power-of-two best-fit search.
Each allocated payload has a header and a footer that contain the block size, and an isAllocated bit for validity.
Each free block of memory also has pointers to the next and previous free block that are of equal or similar size.
segregatedStart contains 28 pointers to linked lists. Each linked list connects free chunks of memory that are of similar size
The first 8 lists of segregatedStart correspond with free lists for chunks of size 0-127 bytes, 128-255 bytes, ..., 896-1023 bytes.
The next  20 lists of segregatedStart correspond with free lists for chunks of size 1024-2047, 2048-4095... 2^29-(2^30-1) bytes.
For each of these size intervals, a free list is created that stores all free chunks of size within that interval.
The best-fit search goes over each linked list, and tries to find a free block that fits the requested size.
malloc() - a free chunk of memory that fits the needed size is found using bestFit(). This block is then cut into two
blocks, one for the allocated block, and the other for the leftover memory. The latter is reinserted in the segregated list.
In case the heap needed to be extended to allocate this memory, and the allocated size is small, we'll allocate multiple
free blocks for future use, to reduce external fragmentation in certain cases. Approximately 4KB bytes worth of free blocks
will be allocated.
free() - the given block is merged with the preceding and following free chunks. This new chunk of memory is then inserted in
the appropriate segregated bin.
realloc() - If possible, realloc will try to extend the currently allocated block to the right. If this is not possible, realloc() will instead use malloc(), copy over the stored data, and then free() the old payload.

