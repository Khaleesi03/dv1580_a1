#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct BlockHeader {
    size_t size;
    int free;
    struct BlockHeader* next;
} BlockHeader;

static void* memory_pool = NULL;
static size_t memory_pool_size = 0;
static size_t memory_used = 0;
static BlockHeader* free_list = NULL;

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

void mem_init(size_t size) {
    memory_pool = malloc(size);
    if (memory_pool == NULL) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        exit(EXIT_FAILURE);
    }
    memory_pool_size = size;
    memory_used = 0;

    free_list = (BlockHeader*)memory_pool;
    free_list->size = size - BLOCK_HEADER_SIZE;
    free_list->free = 1;
    free_list->next = NULL;
}

void* mem_alloc(size_t size) {
    BlockHeader* current = free_list;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            if (current->size > size + BLOCK_HEADER_SIZE) {
                BlockHeader* new_block = (BlockHeader*)((char*)current + BLOCK_HEADER_SIZE + size);
                new_block->size = current->size - size - BLOCK_HEADER_SIZE;
                new_block->free = 1;
                new_block->next = current->next;
                current->size = size;
                current->next = new_block;
            }
            current->free = 0;
            memory_used += size + BLOCK_HEADER_SIZE;
            return (char*)current + BLOCK_HEADER_SIZE;
        }
        current = current->next;
    }
    return NULL;
}

void mem_free(void* block) {
    if (block == NULL) {
        return;
    }
    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    header->free = 1;
    memory_used -= header->size + BLOCK_HEADER_SIZE;

    // Coalesce adjacent free blocks
    BlockHeader* current = free_list;
    while (current != NULL) {
        if (current->free && current->next != NULL && current->next->free) {
            current->size += current->next->size + BLOCK_HEADER_SIZE;
            current->next = current->next->next;
        }
        current = current->next;
    }
}

void* mem_resize(void* block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size);
    }
    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    if (header->size >= size) {
        return block;
    }
    void* new_block = mem_alloc(size);
    if (new_block != NULL) {
        memcpy(new_block, block, header->size);
        mem_free(block);
    }
    return new_block;
}

void mem_deinit() {
    free(memory_pool);
    memory_pool = NULL;
    memory_pool_size = 0;
    memory_used = 0;
    free_list = NULL;
}
