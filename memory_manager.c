#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct BlockHeader {
    size_t offest;              //offset from beginning of memory pool
    size_t size;               // Size of the block (excluding header)
    int free;                 // 1 if the block is free, 0 if allocated
    struct BlockHeader* next;  // Pointer to the next block in the free list
} BlockHeader;

static void* memory_pool = NULL;  // Pointer to the memory pool
static size_t memory_pool_size = 0; // Total size of the memory pool
static BlockHeader* free_list = NULL; // Head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

void mem_init(size_t size) {
    memory_pool = malloc(size);
    if (!memory_pool) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    memory_pool_size = size;

    free_list = (BlockHeader*)memory_pool;
    free_list->size = memory_pool_size - sizeof(BlockHeader);
    free_list->free = 1;
    free_list->next = NULL;
}


void *mem_alloc(size_t size) {
    BlockHeader *current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            size_t remaining = current->size - size - sizeof(BlockHeader);
            if (remaining > sizeof(BlockHeader)) {
                BlockHeader *newBlock = (BlockHeader *)((char *)current + sizeof(BlockHeader) + size);
                newBlock->size = remaining;
                newBlock->free = 1;
                newBlock->next = current->next;
                current->next = newBlock;
                current->size = size;
            }
            current->free = 0;
            return (char *)current + sizeof(BlockHeader);
        }
        current = current->next;
    }
    return NULL;
}

void mem_free(void* block) {
    if (!block) return;
    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    header->free = 1;
    
    // Insert block into free list, maintaining address order
    BlockHeader* current = free_list;
    BlockHeader* prev = NULL;
    while (current && (char*)current < (char*)header) {
        prev = current;
        current = current->next;
    }

    header->next = current;
    if (prev) {
        prev->next = header;
    } else {
        free_list = header;
    }

    // Merge with next block if adjacent
    if (header->next && (char*)header + header->size + BLOCK_HEADER_SIZE == (char*)header->next) {
        header->size += header->next->size + BLOCK_HEADER_SIZE;
        header->next = header->next->next;
    }

    // Merge with previous block if adjacent
    if (prev && (char*)prev + prev->size + BLOCK_HEADER_SIZE == (char*)header) {
        prev->size += header->size + BLOCK_HEADER_SIZE;
        prev->next = header->next;
    }
}

void* mem_resize(void* block, size_t size) {
    if (!block) return mem_alloc(size);

    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    size_t aligned_size = (size + 7) & ~7;

    // Check if next block is free and can be merged
    if (header->next && header->next->free &&
        (char*)header + header->size + BLOCK_HEADER_SIZE == (char*)header->next &&
        header->size + header->next->size + BLOCK_HEADER_SIZE >= aligned_size) {
        header->size += header->next->size + BLOCK_HEADER_SIZE;
        header->next = header->next->next;
        return block;
    }

    void* new_block = mem_alloc(aligned_size);
    if (new_block) {
        memcpy(new_block, block, header->size);
        mem_free(block);
    }
    return new_block;
}

void mem_deinit() {
    if (memory_pool){
        free(memory_pool);
        memory_pool = NULL;
        memory_pool_size = 0;
    }

    BlockHeader* current = free_list;
    while (current){
        BlockHeader* next = current -> next;
        free(current);
        current = next;
    }
    free_list = NULL;
}
