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
    memory_pool = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        exit(EXIT_FAILURE);
    }
    memory_pool_size = size;
    memory_used = 0;

    free_list = (BlockHeader*)memory_pool;
    free_list->size = size - BLOCK_HEADER_SIZE;
    free_list->free = 1;
    free_list->next = NULL;
    if (size > memory_pool_size * 0.2) {
        fprintf(stderr, "Requested memory size exceeds 20%% of the memory pool size\n");
        exit(EXIT_FAILURE);
    }
}

void* mem_alloc(size_t size) {
    BlockHeader* current = free_list;
    BlockHeader* previous = NULL;

    while (current != NULL) {
        if (current->free && current->size >= size) {
            if (current->size >= size + BLOCK_HEADER_SIZE + 1) {
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
        previous = current;
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

    // Add the freed block to the free list if it's not already there
    if (free_list == NULL) {
        free_list = header;
        header->next = NULL;
    } else {
        BlockHeader* current = free_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = header;
        header->next = NULL;
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


