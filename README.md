# Malloc-Implementation
Implementation of C malloc using segregated explicit free lists with an approximate best-fit search.

# Notes

Using the test cases in malloc-tester on my machine (i7-7700HQ, 16GB Ram, Ubuntu 16.04 LTS), both
mallocs are neck-and-neck. On all tries on my computer, my malloc has been within 95% to 105% of my
OS's malloc.

# TODO

- Improve test suite, generate test case from actual real-world malloc() traces.
- Improve malloc() further for real-world malloc() patterns.

# File Documentation

`malloc_tester.c` - Tests my implementation of malloc()/free(), and compares it against the OS's malloc() and free() functions.

`my_malloc.c` - Contains malloc()/free()/realloc() functions implemented using segregated explicit free lists. 

`memlib.c` - Contains a very simple implementation of sbrk(), which is used by my implementation of malloc() to allocate heap memory.

# Details on Malloc implementation
my_malloc.c - contains malloc/free/realloc functions implemented using segregated explicit free lists.

This library implements memory allocation operations by using an approximate power-of-two best-fit
search.

Each allocated payload has a header and a footer that contain the block size, and an isAllocated bit
for validity.

Each free block of memory also has pointers to the next and previous free block that are of equal or
similar size.

segregatedStart contains 28 pointers to linked lists. Each linked list connects free chunks of
memory that are of similar size

The first 8 lists of segregatedStart correspond with free lists for chunks of size 0-127 bytes,
128-255 bytes, ..., 896-1023 bytes.

The next  20 lists of segregatedStart correspond with free lists for chunks of size 1024-2047,
2048-4095... 2^29-(2^30-1) bytes.

For each of these size intervals, a free list is created that stores all free chunks of size within
that interval.

The best-fit search goes over each linked list, and tries to find a free block that fits the
requested size.

malloc() - a free chunk of memory that fits the needed size is found using bestFit(). This block is
then cut into two blocks, one for the allocated block, and the other for the leftover memory. The
latter is reinserted in the segregated list. In case the heap needed to be extended to allocate this
memory, and the allocated size is small, we'll allocate multiple free blocks for future use, to
reduce external fragmentation in certain cases. Approximately 4KB bytes worth of free blocks will be
allocated.

free() - the given block is merged with the preceding and following free chunks. This new chunk of
memory is then inserted in the appropriate segregated bin.

realloc() - If possible, realloc will try to extend the currently allocated block to the right. If
this is not possible, realloc() will instead use malloc(), copy over the stored data, and then
free() the old payload.

# Compiling

To compile this project, run
```
make
```

# Testing

After building the project, execute `malloc_tester` to test the malloc implementation.
There are currently three test cases:

1. Execute malloc() and free() randomly (i.e. with equal probability), and each malloc() call
requests between 20-40 bytes of memory allocated. This test simulates the malloc() and free()
pattern of a linked list by generating very small blocks.

2. Execute malloc() and free() randomly (i.e. with equal probability), and each malloc() call
requests between 100-1000 bytes of memory allocated. This test simulates array allocations of sizes
between 25 to 250 integers.

3. Execute malloc() and free() randomly (i.e. with equal probability), and each malloc() call
requests some power of two between 16 to 16384 bytes of memory allocated. This simulates general OS
usage, with unpredictable byte size requests.

The program uses user input to choose between these test cases. Each time it requests for a block
allocation or deallocation, it calls both my_malloc() (my implementation) and malloc() (the OS
malloc) and calculates how much time it took for each call to finish.
