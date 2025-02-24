#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h> // For mmap and munmap

typedef struct BlockHeader {
    size_t size;               // Size of the block (excluding header)
    int free;                 // 1 if the block is free, 0 if allocated
    struct BlockHeader* next;  // Pointer to the next block in the free list
} BlockHeader;

#define MEMORY_POOL_SIZE 6000 

static void* memory_pool = NULL;  // Pointer to the memory pool
static size_t memory_pool_size = 0; // Total size of the memory pool
static size_t memory_used = 0;      // Amount of memory currently in use
static size_t memory_limit = 0;     // Maximum allowed memory usage
static BlockHeader* free_list = NULL; // Head of the free list

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

// Initialize the memory manager with a given pool size
void mem_init(size_t size) {
    memory_pool = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        fprintf(stderr, "Failed to initialize memory pool\n");
        exit(EXIT_FAILURE);
    }

    memory_pool_size = size;
    memory_limit = size * 1.2; // Set limit to 120% of the pool size
    memory_used = 0;

    // Initialize the free list with one large block
    free_list = (BlockHeader*)memory_pool;
    free_list->size = size - BLOCK_HEADER_SIZE; // Size of the free block
    free_list->free = 1; // Mark as free
    free_list->next = NULL; // No next block
}

void* mem_alloc(size_t size) {
    if (size == 0 || memory_used + size > memory_limit) return NULL; // Handle zero allocation and memory limit

    size_t aligned_size = (size + 7) & ~7; // Align size to 8-byte boundary
    BlockHeader* prev = NULL;
    BlockHeader* current = free_list;
    size_t total_allocation = aligned_size + BLOCK_HEADER_SIZE; // Total size including header

    // Search for a suitable block using the first-fit strategy
    while (current != NULL) {
        if (current->free && current->size >= aligned_size) {
            if (current->size >= total_allocation + BLOCK_HEADER_SIZE) {
                // Split the block
                BlockHeader* new_block = (BlockHeader*)((char*)current + total_allocation);
                new_block->size = current->size - total_allocation;
                new_block->free = 1;
                new_block->next = current->next;
                current->size = aligned_size;
                current->next = new_block;
            }

            current->free = 0; // Mark as allocated
            memory_used += total_allocation;

            // Remove from free list
            if (prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }

            return (char*)current + BLOCK_HEADER_SIZE;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

void mem_free(void* block) {
    if (block == NULL) return;

    BlockHeader* header = (BlockHeader*)((char*)block - BLOCK_HEADER_SIZE);
    header->free = 1;
    memory_used -= header->size + BLOCK_HEADER_SIZE;

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
    if (free_list != NULL) {
        fprintf(stderr, "Memory leak detected: free list is not empty.\n");
    }
    BlockHeader* current = free_list;
    while (current) {
        printf("Free list contains block at %p, size: %zu\n", current, current->size);
        current = current->next;
    }
    munmap(memory_pool, memory_pool_size);
    memory_pool = NULL;
    memory_pool_size = 0;
    memory_used = 0;
}
