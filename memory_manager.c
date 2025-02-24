#include <sys/mman.h>
#include <stdio.h>
#include <stdint.h>

typedef struct BlockHeader {
    size_t size;
    int free;
    struct BlockHeader* next;
} BlockHeader;

static void* memory_pool = NULL;
static size_t memory_pool_size = 0;
static BlockHeader* free_list = NULL;

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

void mem_init(size_t size) {
    memory_pool = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    memory_pool_size = size;
    free_list = (BlockHeader*)memory_pool;
    free_list->size = size - BLOCK_HEADER_SIZE;
    free_list->free = 1;
    free_list->next = NULL;
}

void* mem_alloc(size_t size) {
    BlockHeader* current = free_list;
    BlockHeader* prev = NULL;

    while (current) {
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
            if (prev) prev->next = current->next;
            else free_list = current->next;
            return (char*)current + BLOCK_HEADER_SIZE;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

void mem_free(void* block) {
    if (!block) return;
    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    header->free = 1;
    
    // Merge with next block if it's free
    if (header->next && header->next->free) {
        header->size += header->next->size + BLOCK_HEADER_SIZE;
        header->next = header->next->next;
    }

    // Insert back into free list
    BlockHeader* current = free_list;
    BlockHeader* prev = NULL;
    while (current && current < header) {
        prev = current;
        current = current->next;
    }
    if (prev) {
        prev->next = header;
        header->next = current;
        // Merge with previous block if it's free
        if ((char*)prev + prev->size + BLOCK_HEADER_SIZE == (char*)header) {
            prev->size += header->size + BLOCK_HEADER_SIZE;
            prev->next = header->next;
        }
    } else {
        header->next = free_list;
        free_list = header;
    }
}

void* mem_resize(void* block, size_t size) {
    if (!block) return mem_alloc(size);
    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    if (header->size >= size) return block;
    void* new_block = mem_alloc(size);
    if (new_block) {
        memcpy(new_block, block, header->size);
        mem_free(block);
    }
    return new_block;
}

void mem_deinit() {
    munmap(memory_pool, memory_pool_size);
    memory_pool = NULL;
    memory_pool_size = 0;
    free_list = NULL;
}