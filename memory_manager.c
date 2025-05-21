<<<<<<< HEAD
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
=======
Code
Feedback Overview
AutoTest
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct BlockHeader {
    size_t offset;              //offset from beginning of memory pool
    size_t size;               // Size of the block (excluding header)
    int free;                 // 1 if the block is free, 0 if allocated
    struct BlockHeader* next;  // Pointer to the next block in the free list
} BlockHeader;

static char* memory_pool = NULL;  // Pointer to the memory pool
static size_t memory_pool_size = 0; // Total size of the memory pool
static BlockHeader* free_list = NULL; // Head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

int mem_init(size_t size) {
    if (memory_pool != NULL) return -1;

    memory_pool = malloc(size);
    if (!memory_pool) return -1;

    memory_pool_size = size;

    // First block takes the entire pool, minus space for header
    free_list = (BlockHeader*) memory_pool;
    free_list->offset = sizeof(BlockHeader); // payload starts after header
    free_list->size = size - sizeof(BlockHeader);
    free_list->free = 1;
    free_list->next = NULL;

    return 0;
}

void* mem_alloc(size_t size) {
    if (size == 0) {
        return NULL; // Return NULL if requested size is 0
    }

    BlockHeader* current = free_list; // Start with the head of the free list
    while (current) {
        // Check if the current block is free and large enough
        if (current->free && current->size >= size) {
            size_t total_size = size + sizeof(BlockHeader); // Total size including header
            size_t remaining_size = current->size - size; // Remaining size after allocation
            
            if (remaining_size < sizeof(BlockHeader)) {
                // Not enough space to split, use the whole block
                current->free = 0; // Mark block as allocated
            } else {
                // Enough space to split the block
                current->size = size; // Update the size of the current block
                current->free = 0; // Mark block as allocated
            }

            if (remaining_size >= sizeof(BlockHeader) + 1) {
                // Create a new block inside the current free block
                size_t new_block_offset = current->offset + sizeof(BlockHeader) + size;

                // Initialize the new block header
                BlockHeader* newBlock = (BlockHeader*)(memory_pool + new_block_offset - sizeof(BlockHeader));
                newBlock->offset = new_block_offset; // Set the offset of the new block
                newBlock->size = current->size - size - sizeof(BlockHeader); // Set the size of the new block
                newBlock->free = 1; // Mark the new block as free
                newBlock->next = current->next; // Link the new block to the next block

                // Update the current block's size and next pointer
                current->size = size;
                current->free = 0;
                current->next = newBlock;
            } else {
                // Not enough room to split — use the whole block
                current->free = 0; // Mark block as allocated
            }

            return memory_pool + current->offset; // Return pointer to the allocated memory
        }

        current = current->next; // Move to the next block in the free list
    }

    return NULL; // No suitable block found
}

void mem_free(void* block) {
    if (!block) return; // Early return if the block is NULL

    size_t offset = (char*)block - memory_pool; // Calculate offset of the block in the memory pool

    // Traverse the free list to find the block to free
    BlockHeader* current = free_list;
    BlockHeader* prev = NULL;

    while (current) {
        if (current->offset == offset) {  // If this is the block we need to free
            if (current->free) {  // If the block is already free, return early
                return;
            }
            current->free = 1;  // Mark the block as free

               // Merge with next
               if (current->next && current->next->free) {
                current->size += BLOCK_HEADER_SIZE + current->next->size;
                current->next = current->next->next;
            }

            // Merge with prev
            if (prev && prev->free) {
                prev->size += BLOCK_HEADER_SIZE + current->size;
                prev->next = current->next;
            }
            return;
        }
        prev = current;
        current = current->next;
    }
}

void* mem_resize(void* block, size_t size) {
    if (!block) return mem_alloc(size); // If NULL, allocate new memory.

    BlockHeader* header = free_list;
    while (header && (memory_pool + header->offset != block)) {
        header = header->next;
    }
    if (!header) {
        return NULL; // Block not found in the free list
    }

    // If the requested size is the same or smaller, return the same block
    if (size <= header->size) {
        return block;
    }

    // Check if next block is free and can be merged
    if (header->next && header->next->free &&
        (char*)header + header->size + sizeof(BlockHeader) == (char*)header->next &&
        header->size + header->next->size + sizeof(BlockHeader) >= size) {
        
        header->size += header->next->size + sizeof(BlockHeader);
        header->next = header->next->next;
        return block;
    }

    // Allocate a new block and copy the old data
    void* new_block = mem_alloc(size);
    if (new_block) {
        memcpy(new_block, block, header->size); // Copy only existing data
        mem_free(block);
    }
    return new_block;
}


void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);  // Free the memory pool
    }
    memory_pool = NULL;
    memory_pool_size = 0;
    free_list = NULL;
>>>>>>> d8675c2f0fdc43be70019f38d9fe0d8fcc2aa578
}
