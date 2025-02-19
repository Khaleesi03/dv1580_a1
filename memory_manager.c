#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

typedef struct BlockHeader {
    size_t size;
    int free;
    struct BlockHeader* next;
} BlockHeader;

static void* memory_pool = NULL;
static size_t memory_pool_size = 0;
static size_t memory_used = 0;
static size_t memory_limit = 0;
static BlockHeader* free_list = NULL;

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

void mem_init(size_t size) {
    memory_pool = malloc(size);  // Fix: Corrected the declaration of memory_pool
    if (memory_pool == NULL) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        exit(EXIT_FAILURE);
    }
    memory_pool_size = size;
    memory_limit = size * 1.2;
    memory_used = 0;

    free_list = (BlockHeader*)memory_pool;
    free_list->size = size - BLOCK_HEADER_SIZE;
    free_list->free = 1;
    free_list->next = NULL;
}

void* mem_alloc(size_t size) {
    BlockHeader* current = free_list;
    BlockHeader* previous = NULL;
    size_t total_allocation = size + BLOCK_HEADER_SIZE;

    while (current != NULL) {
        if (current->free && current->size >= total_allocation) {
            if (memory_used + total_allocation > memory_limit) {
                fprintf(stderr, "Memory limit exceeded\n");
                return NULL;
            }
            if (current->size >= total_allocation + BLOCK_HEADER_SIZE) { // Ensure at least 8 bytes of usable space
                BlockHeader* new_block = (BlockHeader*)((char*)current + total_allocation);
                new_block->size = current->size - total_allocation;
                new_block->free = 1;
                new_block->next = current->next;
                current->size = size; // Reduce the current block size
                current->next = new_block;
            }

            current->free = 0;
            memory_used += total_allocation;
            if (current == free_list) {
                free_list = current->next;
            } else {
                previous->next = current->next;
            }
            return (char*)current + BLOCK_HEADER_SIZE;
        }
        previous = current;
        current = current->next;
    }
    return NULL;
}

void mem_free(void* block) {
    if (block == NULL) return;

    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    header->free = 1;  // Mark the block as free
    memory_used -= header->size + BLOCK_HEADER_SIZE;  // Adjust used memory

    // Merge with next block if it's free
    BlockHeader* current = free_list;
    BlockHeader* previous = NULL;
    while (current != NULL && (char*)current < (char*)header) {
        previous = current;
        current = current->next;
    }

    header->next = current;
    if (previous == NULL) {
        free_list = header;
    } else {
        previous->next = header;
    }

    // Try to merge with previous free block
    if (previous && (char*)previous + previous->size + BLOCK_HEADER_SIZE == (char*)header) {
        previous->size += header->size + BLOCK_HEADER_SIZE;
        previous->next = header->next;
        header = previous;
    }

    // Try to merge with next free block
    if (header->next && (char*)header + header->size + BLOCK_HEADER_SIZE == (char*)header->next) {
        header->size += header->next->size + BLOCK_HEADER_SIZE;
        header->next = header->next->next;
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
