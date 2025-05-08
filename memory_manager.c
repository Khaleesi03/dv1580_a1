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

void mem_init(size_t size) {
    memory_pool = (char*)malloc(size);
    if (!memory_pool) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    memory_pool_size = size;

    // Initialize the free list directly in the memory pool
    free_list = (BlockHeader*)malloc(sizeof(BlockHeader));
    if(!free_list){
        printf("malloc of free list failed");
        exit(EXIT_FAILURE);
    }
    free_list->offset = 0;
    free_list->size = size;
    free_list->free = 1;
    free_list->next = NULL;
}

void* mem_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    BlockHeader* current = free_list;

    while (current) {
        if (current->free && current->size >= size) {
            size_t total_size = size + sizeof(BlockHeader);
            size_t remaining_size = current->size - size;

            if (remaining_size >= sizeof(BlockHeader) + 1) {
                // Create a new block inside the current free block
                size_t new_block_offset = current->offset + sizeof(BlockHeader) + size;

                BlockHeader* newBlock = (BlockHeader*)(memory_pool + new_block_offset - sizeof(BlockHeader));
                newBlock->offset = new_block_offset;
                newBlock->size = current->size - size - sizeof(BlockHeader);
                newBlock->free = 1;
                newBlock->next = current->next;

                current->size = size;
                current->free = 0;
                current->next = newBlock;
            } else {
                // Not enough room to split — use the whole block
                current->free = 0;
            }

            return memory_pool + current->offset;
        }

        current = current->next;
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
    memory_pool = NULL;
    memory_pool_size = 0;
    free_list = NULL;
}

