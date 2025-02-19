#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h> // For mmap and munmap

typedef struct BlockHeader {
    size_t size;               // Size of the block (excluding header)
    int free;                 // 1 if the block is free, 0 if allocated
    struct BlockHeader* next;  // Pointer to the next block in the free list
} BlockHeader;

#define size 6000 

static void* memory_pool = NULL;  // Pointer to the memory pool
static size_t memory_pool_size = 0; // Total size of the memory pool
static size_t memory_used = 0;      // Amount of memory currently in use
static size_t memory_limit = 0;     // Maximum allowed memory usage
static BlockHeader* free_list = NULL; // Head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)


// Function to initialize the memory manager
void mem_init(size_t size) {
    memory_pool = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        exit(EXIT_FAILURE);
    }

    memory_pool_size = size;
    memory_limit = size * 1.2; // Set limit to 120% of the pool size
    memory_used = 0;

    // Initialize the free list with one large block
    free_list = (BlockHeader*)memory_pool;
    free_list->size = size - BLOCK_HEADER_SIZE; // Size of the free block
    free_list->free = 1; // Mark as free
    free_list->next = NULL; // No next block
}

// Function to allocate memory
void* mem_alloc(size_t size) {
    if (size == 0) return NULL; // Handle zero allocation

    size_t aligned_size = (size + 7) & ~7; // Align size to 8-byte boundary
    BlockHeader* current = free_list;
    size_t total_allocation = aligned_size + BLOCK_HEADER_SIZE; // Total size including header

    // Search for a suitable block using the first-fit strategy
    while (current != NULL) {
        if (current->free && current->size >= aligned_size) {
            if (memory_used + total_allocation > memory_limit) {
                fprintf(stderr, "Memory limit exceeded\n");
                return NULL;
            }

            // Split the block if it's larger than needed
            if (current->size >= total_allocation + BLOCK_HEADER_SIZE) {
                // Ensure the split block is within valid bounds
                BlockHeader* new_block = (BlockHeader*)((char*)current + total_allocation);
                if ((char*)new_block + BLOCK_HEADER_SIZE > (char*)memory_pool + memory_pool_size) {
                    fprintf(stderr, "Invalid split: new block out of memory bounds\n");
                    return NULL;
                }

                new_block->size = current->size - total_allocation - BLOCK_HEADER_SIZE; // Remaining size
                new_block->free = 1;
                new_block->next = current->next; // Link to next block
                current->size = aligned_size; // Set size of the allocated block
                current->next = new_block; // Link the new block
            }

            current->free = 0; // Mark as allocated
            memory_used += total_allocation; // Update used memory
            return (char*)current + BLOCK_HEADER_SIZE; // Return pointer to usable memory
        }
    
        current = current->next;
    }
    return NULL; // No suitable block found
}



// Function to free allocated memory
void mem_free(void* block) {
    if (block == NULL) return; // Handle null pointer

    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    header->free = 1; // Mark the block as free
    memory_used -= header->size + BLOCK_HEADER_SIZE; // Adjust used memory

    // Merge with next block if it's free
    BlockHeader* current = free_list;
    BlockHeader* previous = NULL;

    while (current != NULL && (char*)current < (char*)header) {
        previous = current;
        current = current->next;
    }

    header->next = current; // Link to next block
    if (previous == NULL) {
        free_list = header; // Update head of free list
    } else {
        previous->next = header; // Link previous to header
    }

    // Try to merge with previous free block
    if (previous && (char*)previous + previous->size + BLOCK_HEADER_SIZE == (char*)header) {
        previous->size += header->size + BLOCK_HEADER_SIZE; // Merge sizes
        previous->next = header->next; // Link to next block
        header = previous; // Update header to point to the merged block
    }

    // Try to merge with next free block
    if (header->next && (char*)header + header->size + BLOCK_HEADER_SIZE == (char*)header->next) {
        header->size += header->next->size + BLOCK_HEADER_SIZE; // Merge sizes
        header->next = header->next->next; // Link to next block
    }

    // Check if the final block is within bounds of the memory pool
    if ((char*)header + header->size + BLOCK_HEADER_SIZE > (char*)memory_pool + memory_pool_size) {
        fprintf(stderr, "Invalid free: block out of memory bounds\n");
    }
}


// Function to resize allocated memory
void* mem_resize(void* block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size); // Allocate new block if null
    }

    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    if (header->size >= size) {
        return block; // No need to resize
    }

    void* new_block = mem_alloc(size); // Allocate new block
    if (new_block != NULL) {
        memcpy(new_block, block, header->size); // Copy data to new block
        mem_free(block); // Free old block
    }
    return new_block; // Return new block
}

// Function to deinitialize the memory manager
void mem_deinit() {
    munmap(memory_pool, memory_pool_size); // Free the memory pool
    memory_pool = NULL; // Reset pointer
    memory_pool_size = 0; // Reset size
    memory_used = 0; // Reset usage
}