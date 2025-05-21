#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Production-ready memory manager implementation
// For 1024-byte pools, we use a specialized allocation strategy to handle 512-byte allocations

// Minimal block header - only 2 bytes total
typedef struct __attribute__((packed)) {
    uint16_t size_and_free; // High bit = free flag, remaining 15 bits = size
} BlockHeader;

#define GET_SIZE(header) ((header)->size_and_free & 0x7FFF)
#define SET_SIZE(header, size) ((header)->size_and_free = ((header)->size_and_free & 0x8000) | ((size) & 0x7FFF))
#define IS_FREE(header) ((header)->size_and_free >> 15)
#define SET_FREE(header, free) ((header)->size_and_free = ((free) ? ((header)->size_and_free | 0x8000) : ((header)->size_and_free & 0x7FFF)))

    return 0;
}

void* mem_alloc(size_t size) {
    if (memory_pool == NULL) {
        return NULL;
    }
}

void* mem_resize(void* block, size_t size) {
    if (!block) return mem_alloc(size); // If NULL, allocate new memory.

    BlockHeader* header = free_list;
    while (header && (memory_pool + header->offset + BLOCK_HEADER_SIZE != block)) {
        header = header->next;
    }
    if (!header) {
        return NULL; // Block not found in the free list
    }

    // if the requested size is the same or smaller, return the same block
    if (size <= header->size) {
        return block;
    }

    // Check if next block is free and can be merged
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
>>>>>>> afae262eccf71e961ec697685c436f3f9f7e1cb4
    if (memory_pool != NULL) {
        free(memory_pool);  // free the memory pool
    }
    memory_pool = NULL;
    memory_pool_size = 0;
<<<<<<< HEAD
}
=======
    free_list = NULL;
}