#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_manager.h"

static void *memory_pool; // Pointer to the memory pool
static size_t pool_size;  // Total size of the memory pool

// Initialization function
void mem_init(size_t size) {
    memory_pool = malloc(size); // Allocate the memory pool
    if (!memory_pool) {
        fprintf(stderr, "Memory allocation failed during initialization.\n");
        exit(EXIT_FAILURE); // Exit if allocation failed
    }

    pool_size = size; // Set the total size of the pool
    memset(memory_pool, 0, pool_size); // Initialize memory to zero
}

// Allocation function
void* mem_alloc(size_t size) {
    if (size == 0 || size + sizeof(size_t) > pool_size) {
        return NULL; // Not enough space or size is zero
    }

    void* current_block = memory_pool; // Start at the beginning of the pool

    // Traverse the pool to find a free block
    while ((char*)current_block < (char*)memory_pool + pool_size) {
        size_t block_size = *(size_t*)current_block; // Get the block size

        // Check if the block is free (block size is 0)
        if (block_size == 0) {
            // Check if the remaining space is sufficient for the new block
            if (pool_size - ((char*)current_block - (char*)memory_pool) >= size + sizeof(size_t)) {
                // Mark block as allocated
                *(size_t*)current_block = size;
                return (char*)current_block + sizeof(size_t); // Return memory after size field
            }
        }
        // Move to the next block in the pool
        current_block = (char*)current_block + block_size + sizeof(size_t);
    }

    return NULL; // No free block found
}

// Deallocation function
void mem_free(void* block) {
    if (block == NULL) {
        return; // Do nothing if the block is null
    }

    // Set the size of the block to 0, indicating it's free
    size_t* block_size_ptr = (size_t*)((char*)block - sizeof(size_t));
    *block_size_ptr = 0; // Mark the block as free
}

// Resize function
void* mem_resize(void* block, size_t new_size) {
    if (block == NULL) {
        return mem_alloc(new_size); // If block is NULL, allocate new memory
    }

    if (new_size == 0) {
        mem_free(block); // Free the block if new size is 0
        return NULL;
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

    return new_block; // Return the new block
}

// Deinitialization function
void mem_deinit() {
    free(memory_pool); // Free the memory pool
    memory_pool = NULL; // Reset the pointer
}
