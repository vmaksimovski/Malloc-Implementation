CC = gcc
CFLAGS = -Wall -O2

OBJS = malloc_tester.o my_malloc.o memlib.o

malloc_tester: $(OBJS)
	$(CC) $(CFLAGS) -o malloc_tester $(OBJS)

malloc_tester.o: malloc_tester.c my_malloc.h memlib.h
memlib.o: memlib.c memlib.h
my_malloc.o: my_malloc.c my_malloc.h memlib.h

clean:
	rm -f *~ *.o malloc_tester


