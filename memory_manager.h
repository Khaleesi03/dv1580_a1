#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static void *memory_pool; //Pointer to the memory pool
static size_t pool_size; //Total size of the memory pool

// Initialization function
//This function sets up the memory pool of the specified size
void mem_init(size_t size)
{
    memory_pool = malloc(size); // Allocate the memory pool
    if (!memory_pool) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE); //Exit if allocation failed
    }

    pool_size = size; //Set the total size of the pool
    memset(memory_pool, 0, pool_size); // Initialize memory to zero
}

// Allocation function
 //This function allocates a block of memory of the specified size
void* mem_alloc(size_t size)
{
    //Ensure tha there is enough space in the pool
    if (size + sizeof(size_t) > pool_size) {
        return NULL; // Not enough space in the pool
    }

    void* current_block = memory_pool; //Start at the beginning of the pool

    //Traverse through the pool to find a free block
    while ((char*)current_block < (char*)memory_pool + pool_size) {
        size_t block_size = *(size_t*)current_block; //Get the block size

        //Check if the block is free (block size is 0)
        if (block_size == 0) { 
            //Check if the remaining space is sufficient for the new block
            if (pool_size - ((char*)current_block - (char*)memory_pool) >= size + sizeof(size_t)) {
                // Mark block as allocated
                *(size_t*)current_block = size;
                return (char*)current_block + sizeof(size_t); //Return memory after size field
            }
        }
        //Move to the next block in the pool
        current_block = (char*)current_block + block_size + sizeof(size_t);
    }

    return NULL; // No free block found
}

// Deallocation function
//Frees a block of memory, marking it as unavailable
void mem_free(void* block)
{
    if (block == NULL) {
        return; //Do nothing if the block is null
    }

    // Set the size of the block to 0, indicating it's free
    size_t* block_size_ptr = (size_t*)((char*)block - sizeof(size_t));
    *block_size_ptr = 0; //Mark the block as free by setting size to 0
}

// Resize function
// Resizes an allocated block to the new size, returning the new block
void* mem_resize(void* block, size_t new_size)
{
    if (block == NULL) {
        return NULL; //Return null if no block to resize
    }

    // Get the original size of the block
    size_t original_size = *(size_t*)((char*)block - sizeof(size_t));

    // Allocate new memory for the block
    void* new_block = mem_alloc(new_size);

    // Copy the data from the old block to the new block
    if (new_block != NULL) {
        memcpy(new_block, block, original_size < new_size ? original_size : new_size);
    }

    // Deallocate the old block
    mem_free(block);

    return new_block; //Return the new block
}
