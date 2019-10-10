#include <stdio.h>

extern int my_malloc_init (void);
extern void *my_malloc (size_t size);
extern void my_free (void *ptr);
extern void *my_realloc(void *ptr, size_t size);
