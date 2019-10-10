#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "my_malloc.h"
#include "memlib.h"

#define NUMBER_OF_ALLOCATIONS 25000
void* my_malloc_allocated_chunks[NUMBER_OF_ALLOCATIONS];
void* system_malloc_allocated_chunks[NUMBER_OF_ALLOCATIONS];

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

void init_allocate(){
    memset(my_malloc_allocated_chunks, 0, sizeof(my_malloc_allocated_chunks));
    memset(system_malloc_allocated_chunks, 0, sizeof(system_malloc_allocated_chunks));
    mem_init();
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

void test_allocation(enum generation_type block_generation_mode){
    clock_t start, end;
    srand(time(0));
    total_time_for_my_malloc = 0.0;
    total_time_for_system_malloc = 0.0;

    init_allocate();
    for(int allocation_number = 1; allocation_number <= NUMBER_OF_ALLOCATIONS; allocation_number++){
        int allocateOrFree = rand() % 2;
        if(allocateOrFree == 0){
            int size = generate_block_size(block_generation_mode);

            start = clock();
            void* my_malloc_new_pointer = my_malloc(size);
            end = clock();

            double time_taken_for_my_malloc = (end - start) / (double) CLOCKS_PER_SEC;
            total_time_for_my_malloc += time_taken_for_my_malloc;

            if(!my_malloc_new_pointer){
                printf("My malloc couldn't allocate a block of size %d\n"
                       "after %d user queries.", size, allocation_number);
                fflush(stdin);
                exit(-1);
            }
            my_malloc_allocated_chunks[next_block_index] = my_malloc_new_pointer;

            start = clock();
            void* system_malloc_new_pointer = malloc(size);
            end = clock();

            double time_taken_for_system_malloc = (end - start) / (double) CLOCKS_PER_SEC;
            total_time_for_system_malloc += time_taken_for_system_malloc;

            if(!system_malloc_new_pointer){
                printf("The system's malloc couldn't allocate a block of size %d\n"
                       "after %d user queries.", size, allocation_number);
                fflush(stdin);
                exit(-1);
            }
            system_malloc_allocated_chunks[next_block_index] = system_malloc_new_pointer;

            next_block_index++;
         } else if(allocateOrFree == 1 && number_of_allocated_blocks != 0){
             int random_block_number = rand() % number_of_allocated_blocks;

             void** my_malloc_ptr = get_nth_nonzero_entry(next_block_index,
                                                          my_malloc_allocated_chunks,
                                                          random_block_number);
             start = clock();
             my_free(*my_malloc_ptr);
             end = clock();
             *my_malloc_ptr = NULL;

             double time_taken_for_my_free = (end - start) / (double) CLOCKS_PER_SEC;
             total_time_for_my_malloc += time_taken_for_my_free;

             void** system_malloc_ptr = get_nth_nonzero_entry(next_block_index,
                                                              system_malloc_allocated_chunks,
                                                              random_block_number);
             start = clock();
             free(*system_malloc_ptr);
             end = clock();
             *system_malloc_ptr = NULL;

             number_of_allocated_blocks--;
             double time_taken_for_system_free = (end - start) / (double) CLOCKS_PER_SEC;
             total_time_for_system_malloc += time_taken_for_system_free;
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
        printf("Scanf expected an int between 1 and 3.");
        exit(-1);
    }

    if(test_case_number == 1){
        test_allocation(SmallBlockSizeGeneration);
    } else if(test_case_number == 2){
        test_allocation(LargeBlockSizeGeneration);
    } else if(test_case_number == 3){
        test_allocation(RandomBlockSizeGeneration);
    } else {
        printf("Found unrecognized test case number %d", test_case_number);
    }

    printf("Test case finished successfully.\n");
    printf("It took your OS's malloc %f ms to run, while my implementation took %f ms.\n"
           "This is within %.2f%% of your OS's malloc!\n",
           1000 * total_time_for_system_malloc, 1000 * total_time_for_my_malloc,
           100 * total_time_for_system_malloc / total_time_for_my_malloc);
}
