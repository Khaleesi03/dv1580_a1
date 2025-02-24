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

#define MEMORY_POOL_SIZE 6000 

static void* memory_pool = NULL;  // Pointer to the memory pool
static size_t memory_pool_size = 0; // Total size of the memory pool
static size_t memory_used = 0;      // Amount of memory currently in use
static size_t memory_limit = 0;     // Maximum allowed memory usage
static BlockHeader* free_list = NULL; // Head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)


// Initialize the memory manager with a given pool size
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

void* mem_alloc(size_t size) {
    if (size == 0) return NULL; // Handle zero allocation

    size_t aligned_size = (size + 7) & ~7; // Align size to 8-byte boundary
    BlockHeader* current = free_list;
    size_t total_allocation = aligned_size + BLOCK_HEADER_SIZE; // Total size including header

    // Search for a suitable block using the first-fit strategy
    while (current != NULL) {
        // Boundary check: Ensure the block is large enough
        if (current->free && current->size >= aligned_size) {
            if (memory_used + total_allocation > memory_limit) {
                fprintf(stderr, "Memory limit exceeded\n");
                return NULL;
            }

            // Split the block if it's larger than needed
            if (current->size >= total_allocation + BLOCK_HEADER_SIZE) {
                // Create a new block for the remaining free memory
                BlockHeader* new_block = (BlockHeader*)((char*)current + total_allocation);
                new_block->size = current->size - total_allocation - BLOCK_HEADER_SIZE; // Remaining size
                new_block->free = 1;
                new_block->next = current->next; // Link to next block
                current->size = aligned_size; // Set size of the allocated block
                current->next = new_block; // Link the new block
            }

            current->free = 0; // Mark as allocated
            memory_used += total_allocation; // Update used memory before allocation

            // Remove current block from free list
            if (current == free_list) {
                free_list = current->next; // Update free list head if current is at the head
            } else {
                BlockHeader* prev = free_list;
                while (prev && prev->next != current) {
                    prev = prev->next;
                }
                if (prev) prev->next = current->next;
            }

            // Return the pointer to the usable memory
            return (char*)current + BLOCK_HEADER_SIZE;
        }
        current = current->next; // Move to next free block
    }

    // No suitable block found
    return NULL;
}

void mem_free(void* block) {
    if (block == NULL) return; // Ignore NULL pointers

    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    printf("Freeing block at %p, size: %zu\n", block, header->size);  // Debug info

    header->free = 1; // Mark the block as free
    memory_used -= header->size + BLOCK_HEADER_SIZE; // Update used memory

    // Insert the block back into the free list (sorted by address)
    BlockHeader* current = free_list;
    BlockHeader* previous = NULL;

    while (current != NULL && (char*)current < (char*)header) {
        previous = current;
        current = current->next;
    }

    header->next = current; // Insert the freed block before `current`
    if (previous == NULL) {
        free_list = header; // Update free list head if necessary
    } else {
        previous->next = header;
    }

    // **Coalescing: Merge with adjacent free blocks if possible**

    // Merge with the next block
    if (header->next && (char*)header + header->size + BLOCK_HEADER_SIZE == (char*)header->next) {
        header->size += header->next->size + BLOCK_HEADER_SIZE; // Increase size
        header->next = header->next->next; // Skip merged block
    }

    // Merge with the previous block
    if (previous && (char*)previous + previous->size + BLOCK_HEADER_SIZE == (char*)header) {
        previous->size += header->size + BLOCK_HEADER_SIZE; // Increase size
        previous->next = header->next; // Skip merged block
        header = previous; // Move header pointer back to merged block
    }

    // **Check if the entire memory is free again**
    if (free_list == header && free_list->next == NULL) {
        // Reset free list to a single large block
        free_list->size = memory_pool_size - BLOCK_HEADER_SIZE;
    }
}



// Function to resize allocated memory
void* mem_resize(void* block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size); // Allocate new block if NULL
    }

    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    size_t aligned_size = (size + 7) & ~7; // Align size to 8-byte boundary
    size_t total_size = aligned_size + BLOCK_HEADER_SIZE; // Total size including header

    // If the current block is large enough, just return it
    if (header->size >= aligned_size) {
        return block;
    }

    // If the current block is too small, allocate a new block
    void* new_block = mem_alloc(aligned_size);
    if (new_block != NULL) {
        memcpy(new_block, block, header->size); // Copy data to new block
        mem_free(block); // Free old block
    }

    return new_block; // Return new block
}

// Function to deinitialize the memory manager
void mem_deinit() {
    if (free_list != NULL) {
        fprintf(stderr, "Memory leak detected: free list is not empty.\n");
        // Optionally iterate over free_list to see unfreed blocks
    }
    BlockHeader* current = free_list;
    while (current != NULL) {
        printf("Free list contains block at %p, size: %zu\n", current, current->size);
        current = current->next;
    }
    munmap(memory_pool, memory_pool_size); // Free the memory pool
    memory_pool = NULL; // Reset pointer
    memory_pool_size = 0; // Reset size
    memory_used = 0; // Reset usage
}
