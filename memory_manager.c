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
        BlockHeader* current = free_list;
        while (current) {
            if (current->free) {
                return memory_pool + current->offset;
            }
            current = current->next;
        }
        return NULL;
    }

    BlockHeader* current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size > size) { 
                // Split the block correctly
                BlockHeader* newBlock = (BlockHeader*)malloc(sizeof(BlockHeader));
                if(!newBlock){
                    return NULL;
                }
                newBlock->offset = current->offset + size;
                newBlock->size = current->size - size - sizeof(BlockHeader);  // Ensure size is correct after splitting
                newBlock->free = 1;
                newBlock->next = current->next;

                current->size = size;
                current->free = 0;
                current->next = newBlock;
            } else {
                current->free = 0;
            }
            return memory_pool + current->offset;
        }
        current = current->next;
    }

    return NULL;
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

            // Merge with the next block if it's free
            if (current->next && current->next->free) {
                BlockHeader* nextBlock = current->next;
                current->size += nextBlock->size + sizeof(BlockHeader); // Add the size of the next block
                current->next = nextBlock->next; // Update the link to skip the next block
                free(nextBlock); // Now it's safe to free the next block
            }

            // Merge with the previous block if it's free
            if (prev && prev->free) {
                prev->size += current->size + sizeof(BlockHeader); // Add the size of the current block
                prev->next = current->next; // Update the previous block's next pointer
                free(current); // Now it's safe to free the current block
                return;  // After merging with the previous block, we are done
            }

            return; // If no merging was done, we're done
        }
        prev = current;
        current = current->next;
    }
}




void* mem_resize(void* block, size_t size) {
    if (!block) return mem_alloc(size); // If NULL, allocate new memory.

    BlockHeader* header = (BlockHeader*)((char*)block - sizeof(BlockHeader));

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

int main() {
    // Initialize memory pool with 6000 bytes
    mem_init(6000);
    printf("Memory pool initialized with 6000 bytes\n");

    // Allocate 6000 bytes
    void* block1 = mem_alloc(6000);
    if (block1) {
        printf("Allocated 6000 bytes at %p\n", block1);
    } else {
        printf("Memory allocation failed\n");
    }

    // Free the allocated block
    mem_free(block1);
    printf("Freed block1\n");

    // Deinitialize memory manager
    mem_deinit();
    printf("Memory deinitialized\n");

    return 0;
}