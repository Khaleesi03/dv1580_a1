#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct BlockHeader {
    size_t size;               // Size of the block (excluding header)
    int free;                 // 1 if the block is free, 0 if allocated
    struct BlockHeader* next;  // Pointer to the next block in the free list
} BlockHeader;

static char* memory_pool = NULL;  // Pointer to the memory pool
static size_t memory_pool_size = 0; // Total size of the memory pool
static BlockHeader* free_list = NULL; // Head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

int mem_init(size_t size) {
    if (memory_pool != NULL) {
        return -1;
    }

    memory_pool = malloc(size);
    if (!memory_pool) {
        return -1;
    }

    memory_pool_size = size;

    // First block takes the entire pool, minus space for header
    free_list = (BlockHeader*) memory_pool;
    free_list->size = size + BLOCK_HEADER_SIZE; // Correct size including header
    free_list->free = 1;
    free_list->next = NULL;

    return 0;
}

void* mem_alloc(size_t size) {
    if (size == 0) {
        // Return pointer to the first free block's data area if available
        if (free_list && free_list->free) {
            return (char*)free_list + sizeof(BlockHeader);
        } else {
            // If no free block, return NULL as fallback
            return NULL;
        }
    }

    BlockHeader* current = free_list;

    while (current) {
        // Check if the current block size is sufficient for allocation
        if (!current->free || current->size < size + sizeof(BlockHeader)) {
            current = current->next;
            continue;
        }

        if (current->free && current->size >= size + sizeof(BlockHeader)) {
            size_t total_size = size + sizeof(BlockHeader);
            size_t remaining_size = current->size - total_size;

            if (remaining_size >= sizeof(BlockHeader) + 1) {
                BlockHeader* newBlock = (BlockHeader*)((char*)current + total_size);
                newBlock->size = remaining_size + BLOCK_HEADER_SIZE; // Adjust size to include header correctly
                newBlock->free = 1;
                newBlock->next = current->next;

                current->size = size;
                current->free = 0;
                current->next = newBlock;
            } else {
                current->free = 0;
            }

            return (char*)current + sizeof(BlockHeader);
        }

        current = current->next;
    }

    return NULL;
}

void mem_free(void* block) {
    if (!block) {
        return;
    }

    BlockHeader* current = free_list;
    BlockHeader* prev = NULL;

    while (current) {
        if ((char*)current + sizeof(BlockHeader) == block) {
            if (current->free) {
                return;
            }
            current->free = 1;

            // Merge with next block if free
            while (current->next && current->next->free) {
                current->size += BLOCK_HEADER_SIZE + current->next->size;
                current->next = current->next->next;
            }

            // Merge with previous block if free
            if (prev && prev->free) {
                prev->size += BLOCK_HEADER_SIZE + current->size;
                prev->next = current->next;
                current = prev; // Update current to prev after merging
            }

            // Update free_list if needed
            if (free_list == NULL || (char*)free_list > (char*)current) {
                free_list = current;
            }

            return;
        }
        prev = current;
        current = current->next;
    }
}

void* mem_resize(void* block, size_t size) {
    if (!block) {
        return mem_alloc(size);
    }

    BlockHeader* header = free_list;
    while (header && ((char*)header + sizeof(BlockHeader) != block)) {
        header = header->next;
    }
    if (!header) {
        return NULL;
    }

    if (size <= header->size) {
        return block;
    }

    if (header->next && header->next->free &&
        (char*)header + sizeof(BlockHeader) + header->size == (char*)header->next &&
        header->size + header->next->size + sizeof(BlockHeader) >= size) {
        
        header->size += header->next->size + sizeof(BlockHeader);
        header->next = header->next->next;
        return block;
    }

    void* new_block = mem_alloc(size);
    if (new_block) {
        memcpy(new_block, block, header->size);
        mem_free(block);
    }
    return new_block;
}

void mem_deinit() {
    if (memory_pool != NULL) {
        printf("mem_deinit: deinitializing memory pool\n");
        free(memory_pool);  // Free the memory pool
    } else {
        printf("mem_deinit: memory pool is already NULL\n");
    }
    memory_pool = NULL;
    memory_pool_size = 0;
    free_list = NULL;
}
