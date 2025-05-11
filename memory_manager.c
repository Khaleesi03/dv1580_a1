#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct BlockHeader {
    size_t size;   // Size of the block (excluding header)
    int free;      // 1 if the block is free, 0 if allocated
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

    BlockHeader* initial = (BlockHeader*)memory_pool;
    initial->size = size - sizeof(BlockHeader);
    initial->free = 1;

    return 0;
}


void* mem_alloc(size_t size) {
    if (size == 0 || memory_pool == NULL) {
        return NULL;
    }

    BlockHeader* current = (BlockHeader*)memory_pool;
    char* pool_end = memory_pool + memory_pool_size;

        while ((char*)current + sizeof(BlockHeader) <= pool_end) {
        if (current->free && current->size >= size) {
            size_t remaining_size = current->size - size;

            if (remaining_size >= sizeof(BlockHeader) + 1) {
                // Split block
                BlockHeader* newBlock = (BlockHeader*)((char*)current + sizeof(BlockHeader) + size);
                newBlock->size = remaining_size - sizeof(BlockHeader);
                newBlock->free = 1;

                current->size = size;
                current->free = 0;
            } else {
                // Use entire block
                current->free = 0;
            }

            return (char*)current + sizeof(BlockHeader);
        }

        current = (BlockHeader*)((char*)current + sizeof(BlockHeader) + current->size);
    }

    return NULL; // No suitable block found
}


void mem_free(void* block) {
    if (!block || memory_pool == NULL) return;

    BlockHeader* current = (BlockHeader*)memory_pool;
    char* pool_end = memory_pool + memory_pool_size;

    while ((char*)current < pool_end) {
        void* user_ptr = (char*)current + sizeof(BlockHeader);
        if (user_ptr == block) {
            current->free = 1;
            return;
        }

        current = (BlockHeader*)((char*)current + sizeof(BlockHeader) + current->size);
    }
}


void* mem_resize(void* block, size_t size) {
    if (!block) return mem_alloc(size);
    if (size == 0) return NULL;

    BlockHeader* current = (BlockHeader*)memory_pool;
    char* pool_end = memory_pool + memory_pool_size;

    while ((char*)current < pool_end) {
        void* user_ptr = (char*)current + sizeof(BlockHeader);
        if (user_ptr == block) {
            if (size <= current->size) {
                return block;  // fits in current block
            }

            // Try to merge with next block if it's free
            BlockHeader* next = (BlockHeader*)((char*)current + sizeof(BlockHeader) + current->size);
            if ((char*)next < pool_end && next->free &&
                current->size + sizeof(BlockHeader) + next->size >= size) {

                current->size += sizeof(BlockHeader) + next->size;
                current->free = 0;
                return block;
            }

            // Otherwise, allocate new block and copy
            void* new_block = mem_alloc(size);
            if (new_block) {
                memcpy(new_block, block, current->size);
                mem_free(block);
            }
            return new_block;
        }

        current = (BlockHeader*)((char*)current + sizeof(BlockHeader) + current->size);
    }

    return NULL;
}



void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);  // Free the memory pool
    }
    memory_pool = NULL;
    memory_pool_size = 0;
    free_list = NULL;
}

