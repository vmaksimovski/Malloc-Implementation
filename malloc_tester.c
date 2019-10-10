#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "my_malloc.h"
#include "memlib.h"

#define MAX_NUMBER_OF_QUERIES 100000
#define MAXMALLOCSIZE 16384

void* my_malloc_allocated_chunks[MAX_NUMBER_OF_QUERIES];
void* system_malloc_allocated_chunks[MAX_NUMBER_OF_QUERIES];

int number_of_allocated_blocks = 0;
int next_block_index = 0;

enum generation_type {
    SmallBlockSizeGeneration,
    LargeBlockSizeGeneration,
    RandomBlockSizeGeneration
};

int generate_block_size(enum generation_type block_generation_mode){
    if(block_generation_mode == SmallBlockSizeGeneration){
        // generate blocks of size 20...39
        return 20 + rand() % 20;
    } else if(block_generation_mode == LargeBlockSizeGeneration){
        // generate blocks of size 100...1000 with step sizes 100
        return (rand() % 10 + 1) * 100;
    } else if(block_generation_mode == RandomBlockSizeGeneration){
        // generate blocks of size 2^n for n between 4 and 14 (16, 32, 64...16384)
        return 1 << (rand() % 11 + 4);
    }

    return 0;
}

void** get_nth_nonzero_entry(int array_size, void* array[], int n){
    if(n >= array_size){
        printf("Test module tried to find block #%d, but array size is %d.", n, array_size);
        exit(-1);
    }

    int current_block = -1;
    for(int i = 0; i < array_size; i++){
        if(array[i]){
            current_block++;

            if(current_block == n){
                return &(array[i]);
            }
        }
    }

    return 0;
}

void init_allocate(int number_of_queries){
    memset(my_malloc_allocated_chunks, 0, sizeof(my_malloc_allocated_chunks));
    memset(system_malloc_allocated_chunks, 0, sizeof(system_malloc_allocated_chunks));

    // to ensure malloc() won't use more memory than sbrk() can handle, we make sbrk() allocate
    // enough memory to accommodate the worst case for malloc()
    mem_init(number_of_queries * MAXMALLOCSIZE);
    my_malloc_init();
}

void dealloc(){
    for(int i = 0; i < next_block_index; i++){
        if(my_malloc_allocated_chunks[i]){
            my_free(my_malloc_allocated_chunks[i]);
        	my_malloc_allocated_chunks[i] = NULL;
        }
        if(system_malloc_allocated_chunks[i]){
            free(system_malloc_allocated_chunks[i]);
            system_malloc_allocated_chunks[i] = NULL;
        }
    }
    mem_deinit();
}

double total_time_for_system_malloc = 0.0;
double total_time_for_my_malloc = 0.0;

void test_allocation(enum generation_type block_generation_mode, int number_of_queries){
    clock_t start, end;
    srand(time(0));
    total_time_for_my_malloc = 0.0;
    total_time_for_system_malloc = 0.0;

    init_allocate(number_of_queries);
    for(int allocation_number = 1; allocation_number <= number_of_queries; allocation_number++){
        int allocateOrFree = rand() % 2;
        if(allocateOrFree == 0){
            int size = generate_block_size(block_generation_mode);

            start = clock();
            void* my_malloc_new_pointer = my_malloc(size);
            end = clock();
            double time_taken_for_my_malloc = (end - start) / (double) CLOCKS_PER_SEC;

            start = clock();
            void* system_malloc_new_pointer = malloc(size);
            end = clock();
            double time_taken_for_system_malloc = (end - start) / (double) CLOCKS_PER_SEC;

            if(!my_malloc_new_pointer){
                printf("My malloc couldn't allocate a block of size %d\n"
                       "after %d user queries.", size, allocation_number);
                fflush(stdin);
                exit(-1);
            }
            if(!system_malloc_new_pointer){
                printf("The system's malloc couldn't allocate a block of size %d\n"
                       "after %d user queries.", size, allocation_number);
                fflush(stdin);
                exit(-1);
            }

            my_malloc_allocated_chunks[next_block_index] = my_malloc_new_pointer;
            system_malloc_allocated_chunks[next_block_index] = system_malloc_new_pointer;

            total_time_for_my_malloc += time_taken_for_my_malloc;
            total_time_for_system_malloc += time_taken_for_system_malloc;

            next_block_index++;
         } else if(allocateOrFree == 1 && number_of_allocated_blocks != 0){
             int random_block_number = rand() % number_of_allocated_blocks;

             void** my_malloc_ptr = get_nth_nonzero_entry(next_block_index,
                                                          my_malloc_allocated_chunks,
                                                          random_block_number);
             void** system_malloc_ptr = get_nth_nonzero_entry(next_block_index,
                                                              system_malloc_allocated_chunks,
                                                              random_block_number);
             start = clock();
             my_free(*my_malloc_ptr);
             end = clock();
             double time_taken_for_my_free = (end - start) / (double) CLOCKS_PER_SEC;

             start = clock();
             free(*system_malloc_ptr);
             end = clock();
             double time_taken_for_system_free = (end - start) / (double) CLOCKS_PER_SEC;

             *my_malloc_ptr = NULL;
             *system_malloc_ptr = NULL;

             total_time_for_my_malloc += time_taken_for_my_free;
             total_time_for_system_malloc += time_taken_for_system_free;
             number_of_allocated_blocks--;
        }
    }

    dealloc();
}

int main(){
    printf("Welcome to the Malloc tester!\n"
           "This compares your OS's malloc and my implementation of malloc.\n"
           "----------------------------------\n"
           "Enter a test case to try out:\n"
           "1. Randomly generate/free blocks of size ~20-40 bytes.\n"
           "2. Randomly generate/free of larger blocks(100-900 bytes).\n"
           "3. Generate/free blocks of random sizes.\n");

    int test_case_number = 0;
    if(!scanf("%d", &test_case_number)){
        printf("ERR: Scanf expected an integer.");
        exit(-1);
    }

    printf("How many add/remove requests should be done?\n"
    	   "Minimum is 0, and maximum is %d\n", MAX_NUMBER_OF_QUERIES);
    int number_of_queries = 0;
    if(!scanf("%d", &number_of_queries)){
        printf("ERR: Scanf expected an integer.");
        exit(-1);
    }

    if(!(0 <= number_of_queries && number_of_queries <= MAX_NUMBER_OF_QUERIES)){
    	printf("Received number of queries %d outside range 1...%d.\n", number_of_queries,
    																    MAX_NUMBER_OF_QUERIES);
    }

    if(test_case_number == 1){
        test_allocation(SmallBlockSizeGeneration, number_of_queries);
    } else if(test_case_number == 2){
        test_allocation(LargeBlockSizeGeneration, number_of_queries);
    } else if(test_case_number == 3){
        test_allocation(RandomBlockSizeGeneration, number_of_queries);
    } else {
        printf("Found unrecognized test case number %d", test_case_number);
    }

    printf("Test case finished successfully.\n");
    printf("It took your OS's malloc %f ms to run, while my implementation took %f ms.\n"
           "This is within %.2f%% of your OS's malloc!\n",
           1000 * total_time_for_system_malloc, 1000 * total_time_for_my_malloc,
           100 * total_time_for_system_malloc / total_time_for_my_malloc);
}
