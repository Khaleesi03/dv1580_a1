#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct BlockHeader {
    size_t offset;              // offset from beginning of memory pool
    size_t size;                // size of the block (excluding header)
    int free;                   // 1 if the block is free, 0 if allocated
    struct BlockHeader* next;   // pointer to the next block in the free list
} BlockHeader;

static char* memory_pool = NULL;  // pointer to the memory pool
static size_t memory_pool_size = 0; // total size of the memory pool
static BlockHeader* free_list = NULL; // head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

static size_t allocated_blocks = 0; // track number of allocated blocks
static const float MAX_METADATA_OVERHEAD = 0.2f; // 20% max overhead

int mem_init(size_t size) {
    if (memory_pool != NULL) return -1;

    memory_pool = malloc(size);
    if (!memory_pool) return -1;

    memory_pool_size = size;

    // first block takes the entire pool minus header size
    free_list = (BlockHeader*) memory_pool;
    free_list->offset = 0; // payload starts at beginning of pool
    free_list->size = size - BLOCK_HEADER_SIZE; // adjust size to exclude header
    free_list->free = 1;
    free_list->next = NULL;

    return 0;
}

void* mem_alloc(size_t size) {
    if (size == 0) {
        return NULL; // return NULL if requested size is 0
    }

    // check metadata overhead limit
    size_t current_metadata_size = allocated_blocks * BLOCK_HEADER_SIZE;
    if (current_metadata_size + BLOCK_HEADER_SIZE > (size_t)(memory_pool_size * MAX_METADATA_OVERHEAD)) {
        // exceeds metadata overhead limit
        return NULL;
    }

    BlockHeader* current = free_list; // start with the head of the free list
    while (current) {
        // debug print
        printf("Checking block at offset %zu, size %zu, free %d\n", current->offset, current->size, current->free);

        // check if the current block is free and large enough
        if (current->free && current->size >= size) {
            printf("Allocating %zu bytes from block at offset %zu\n", size, current->offset);

            size_t remaining_size = current->size - size - BLOCK_HEADER_SIZE; // remaining size after allocation

            if (current->size == size) {
                // exact fit, no splitting needed
                current->free = 0; // mark block as allocated
            } else if (remaining_size >= 1) {
                // enough space to split the block
                // create a new block inside the current free block
                size_t new_block_offset = current->offset + BLOCK_HEADER_SIZE + size;

                // initialize the new block header
                BlockHeader* newBlock = (BlockHeader*)(memory_pool + new_block_offset);
                newBlock->offset = new_block_offset; // set the offset of the new block
                newBlock->size = remaining_size; // set the size of the new block
                newBlock->free = 1; // mark the new block as free
                newBlock->next = current->next; // link the new block to the next block

                // update the current block's size and next pointer
                current->size = size;
                current->free = 0;
                current->next = newBlock;

                printf("Split block at offset %zu into allocated size %zu and free size %zu\n", current->offset, current->size, newBlock->size);
            } else {
                // not enough room to split — use the whole block
                current->free = 0; // mark block as allocated
            }

            allocated_blocks++; // increment allocated blocks count
            return memory_pool + current->offset + BLOCK_HEADER_SIZE; // return pointer to the allocated memory payload
        }

        current = current->next; // move to the next block in the free list
    }

    return NULL; // no suitable block found
}

void mem_free(void* block) {
    if (!block) return; // early return if the block is NULL

    size_t offset = (char*)block - memory_pool - BLOCK_HEADER_SIZE; // calculate offset of the block header in the memory pool

    // traverse the free list to find the block to free
    BlockHeader* current = free_list;
    BlockHeader* prev = NULL;

    while (current) {
        if (current->offset == offset) {  // if this is the block we need to free
            if (current->free) {  // if the block is already free, return early
                printf("Warning: Double free detected at offset %zu\n", offset);
                return;
            }
            current->free = 1;  // mark the block as free
            printf("Freeing block at offset %zu, size %zu\n", current->offset, current->size);

            // merge with next
            if (current->next && current->next->free) {
                printf("Merging with next block at offset %zu, size %zu\n", current->next->offset, current->next->size);
                current->size += BLOCK_HEADER_SIZE + current->next->size;
                current->next = current->next->next;
            }

            // merge with prev
            if (prev && prev->free) {
                printf("Merging with previous block at offset %zu, size %zu\n", prev->offset, prev->size);
                prev->size += BLOCK_HEADER_SIZE + current->size;
                prev->next = current->next;
            }
            allocated_blocks--; // decrement allocated blocks count
            return;
        }
        prev = current;
        current = current->next;
    }
}

void* mem_resize(void* block, size_t size) {
    if (!block) return mem_alloc(size); // if NULL, allocate new memory.

    BlockHeader* header = free_list;
    while (header && (memory_pool + header->offset + BLOCK_HEADER_SIZE != block)) {
        header = header->next;
    }
    if (!header) {
        return NULL; // block not found in the free list
    }

    // if the requested size is the same or smaller, return the same block
    if (size <= header->size) {
        return block;
    }

    // check if next block is free and can be merged
    if (header->next && header->next->free &&
        (char*)header + BLOCK_HEADER_SIZE + header->size == (char*)header->next &&
        header->size + header->next->size + BLOCK_HEADER_SIZE >= size) {
        
        header->size += header->next->size + BLOCK_HEADER_SIZE;
        header->next = header->next->next;
        return block;
    }

    // allocate a new block and copy the old data
    void* new_block = mem_alloc(size);
    if (new_block) {
        memcpy(new_block, block, header->size); // copy only existing data
        mem_free(block);
    }
    return new_block;
}

void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);  // free the memory pool
    }
    memory_pool = NULL;
    memory_pool_size = 0;
    free_list = NULL;
}
