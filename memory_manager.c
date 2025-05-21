#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Structure for each memory block header
typedef struct BlockHeader {
    size_t size;                 // Size of the memory block (excluding header)
    int free;                    // 1 if block is free, 0 if allocated
    struct BlockHeader* next;    // Pointer to next free block
} BlockHeader;

// Static global variables for memory manager state
static char* memory_pool = NULL;       // Start of allocated memory pool
static size_t memory_pool_size = 0;    // Total size of the pool
static BlockHeader* free_list = NULL;  // Head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)  // Convenience macro for header size

// Initialize the memory pool with a given size
int mem_init(size_t size) {
    if (memory_pool != NULL) {
        // Already initialized, prevent reinitialization
        return -1;
    }

    // Allocate raw memory pool
    memory_pool = malloc(size);
    if (!memory_pool) {
        // Allocation failed
        return -1;
    }

    memory_pool_size = size;

    // Set up the first (and only) free block that spans the entire pool
    free_list = (BlockHeader*) memory_pool;
    free_list->size = size + BLOCK_HEADER_SIZE;  // Account for header size
    free_list->free = 1;                          // Mark block as free
    free_list->next = NULL;                       // No next block

    return 0;  // Success
}

// Allocate a memory block of a given size
void* mem_alloc(size_t size) {
    if (size == 0) {
        // Zero-byte allocation: return first free block if available
        if (free_list && free_list->free) {
            return (char*)free_list + BLOCK_HEADER_SIZE;
        } else {
            return NULL;
        }
    }

    BlockHeader* current = free_list;

    // Traverse free list to find a large enough block
    while (current) {
        // Skip if block is not free or too small
        if (!current->free || current->size < size + BLOCK_HEADER_SIZE) {
            current = current->next;
            continue;
        }

        // Suitable block found
        size_t total_size = size + BLOCK_HEADER_SIZE;
        size_t remaining_size = current->size - total_size;

        if (remaining_size >= BLOCK_HEADER_SIZE + 1) {
            // Split current block into allocated and remaining free block
            BlockHeader* newBlock = (BlockHeader*)((char*)current + total_size);
            newBlock->size = remaining_size + BLOCK_HEADER_SIZE;  // Include header
            newBlock->free = 1;
            newBlock->next = current->next;

            // Adjust current block
            current->size = size;  // Shrink to requested size
            current->free = 0;     // Mark as allocated
            current->next = newBlock;
        } else {
            // Not enough space to split — allocate whole block
            current->free = 0;
        }

        // Return pointer to usable memory after header
        return (char*)current + BLOCK_HEADER_SIZE;
    }

    // No suitable block found
    return NULL;
}

// Free a previously allocated block
void mem_free(void* block) {
    if (!block) {
        return;  // Null pointer check
    }

    BlockHeader* current = free_list;
    BlockHeader* prev = NULL;

    // Traverse free list to locate the block header corresponding to the data pointer
    while (current) {
        // Check if current block matches the pointer
        if ((char*)current + BLOCK_HEADER_SIZE == block) {
            if (current->free) {
                return;  // Already free, ignore
            }

            current->free = 1;  // Mark block as free

            // Merge with next adjacent free blocks
            while (current->next && current->next->free) {
                current->size += BLOCK_HEADER_SIZE + current->next->size;
                current->next = current->next->next;
            }

            // Merge with previous block if also free
            if (prev && prev->free) {
                prev->size += BLOCK_HEADER_SIZE + current->size;
                prev->next = current->next;
                current = prev;  // Update current to merged block
            }

            // Update head of free list if necessary
            if (free_list == NULL || (char*)free_list > (char*)current) {
                free_list = current;
            }

            return;  // Done
        }

        prev = current;
        current = current->next;
    }
}

// Resize an allocated memory block to a new size
void* mem_resize(void* block, size_t size) {
    if (!block) {
        // Equivalent to malloc
        return mem_alloc(size);
    }

    // Locate the header for this block
    BlockHeader* header = free_list;
    while (header && ((char*)header + BLOCK_HEADER_SIZE != block)) {
        header = header->next;
    }

    if (!header) {
        return NULL;  // Block not found
    }

    if (size <= header->size) {
        // Requested size fits in existing block
        return block;
    }

    // Check if we can merge with next free block
    if (header->next && header->next->free &&
        (char*)header + BLOCK_HEADER_SIZE + header->size == (char*)header->next &&
        header->size + header->next->size + BLOCK_HEADER_SIZE >= size) {
        
        // Extend current block into next block
        header->size += header->next->size + BLOCK_HEADER_SIZE;
        header->next = header->next->next;
        return block;
    }

    // Fallback: allocate new block and copy old data
    void* new_block = mem_alloc(size);
    if (new_block) {
        memcpy(new_block, block, header->size);  // Copy old data
        mem_free(block);  // Free old block
    }

    return new_block;
}

// Deinitialize memory pool, releasing all memory
void mem_deinit() {
    if (memory_pool != NULL) {
        printf("mem_deinit: deinitializing memory pool\n");
        free(memory_pool);  // Free entire pool
    } else {
        printf("mem_deinit: memory pool is already NULL\n");
    }

    // Reset global variables
    memory_pool = NULL;
    memory_pool_size = 0;
    free_list = NULL;
}
