#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * @struct MemBlock
 * @brief Structure to track a block of memory in the external memory manager.
 *
 * @var offset Offset of the block from the start of the memory pool.
 * @var size Size of the memory block in bytes.
 * @var is_block_free Flag indicating if the block is free (1) or allocated (0).
 * @var next Pointer to the next memory block in the linked list.
 */
typedef struct MemBlock {
    size_t offset;          /**< Offset from the start of the memory pool */
    size_t size;            /**< Size of the memory block */
    int is_block_free;      /**< 1 if block is free, 0 if allocated */
    struct MemBlock* next;  /**< Pointer to next block in the list */
} MemBlock;

/** Head pointer to the linked list of memory blocks */
static MemBlock* mem_block_list = NULL;

/** Pointer to the start of the allocated memory pool */
static char* mem_pool = NULL;

/** Total size of the memory pool in bytes */
static size_t mem_pool_size = 0;

/**
 * @brief Initialize the memory pool with a given size.
 *
 * Allocates the memory pool and sets up the initial free block covering the entire pool.
 *
 * @param size Size of the memory pool to allocate in bytes.
 * @return 0 on success, -1 on failure (e.g., already initialized or malloc failure).
 */

int mem_init(size_t size) {
    if (mem_pool != NULL) return -1;

    mem_pool = malloc(size);
    if (!mem_pool) return -1;

    mem_pool_size = size;

    // Setup initial free block covering entire pool
    mem_block_list = (MemBlock*)malloc(sizeof(MemBlock));
    if (!mem_block_list) return -1;

    mem_block_list->offset = 0;
    mem_block_list->size = size;
    mem_block_list->is_block_free = 1;
    mem_block_list->next = NULL;

    return 0;
}


/**
 * @brief Allocate a memory block of a given size from the memory pool.
 *
 * If size is 0, returns the first free block's address.
 * Otherwise, finds a free block large enough to satisfy the request.
 * If the block is larger than needed, splits it into allocated and free parts.
 *
 * @param size Size of the memory block to allocate in bytes.
 * @return Pointer to the allocated memory block, or NULL if allocation fails.
 */
void* mem_alloc(size_t size) {
    if (!mem_pool) return NULL;

    if (size == 0) {
        MemBlock* current_block = mem_block_list;
        while (current_block){
            if (current_block->is_block_free){
                return mem_pool + current_block->offset;
            }
            current_block = current_block->next;
        }
        return NULL;
    }

    MemBlock* current_block = mem_block_list;

    while (current_block) {
        if (current_block->is_block_free && current_block->size >= size) {
            // If block is bigger than needed, split it
            if (current_block->size > size) {
                MemBlock* new_block = (MemBlock*)malloc(sizeof(MemBlock));
                if (!new_block) return NULL;

                new_block->offset = current_block->offset + size;
                new_block->size = current_block->size - size;
                new_block->is_block_free = 1;
                new_block->next = current_block->next;

                current_block->size = size;
                current_block->is_block_free = 0;
                current_block->next = new_block;
            } else {
                current_block->is_block_free = 0;  // Use entire block
            }

            return mem_pool + current_block->offset;
        }

        current_block = current_block->next;
    }

    return NULL;  // No suitable block found
}

/**
 * @brief Free a previously allocated memory block.
 *
 * Marks the block as free and merges with adjacent free blocks if possible.
 *
 * @param ptr Pointer to the memory block to free.
 */
void mem_free(void* ptr) {
    if (!ptr || !mem_pool) return;

    size_t offset = (char*)ptr - mem_pool;
    MemBlock* current_block = mem_block_list;
    MemBlock* previous_block = NULL;

    while (current_block) {
        if (current_block->offset == offset) {
            if (current_block->is_block_free) return;  // Already free

            current_block->is_block_free = 1;

            // Merge with next block if it is free
            if (current_block->next && current_block->next->is_block_free) {
                MemBlock* next_block = current_block->next;
                current_block->size += next_block->size;
                current_block->next = next_block->next;
                free(next_block);
            }

            // Merge with previous block if it is free
            if (previous_block && previous_block->is_block_free) {
                previous_block->size += current_block->size;
                previous_block->next = current_block->next;
                free(current_block);
            }

            return;
        }

        previous_block = current_block;
        current_block = current_block->next;
    }
}


// Resize an allocated memory block to a new size
void* mem_resize(void* ptr, size_t size) {
    if (!ptr) return mem_alloc(size);
    if (size == 0) {
        mem_free(ptr);
        return NULL;
    }

    size_t offset = (char*)ptr - mem_pool;
    MemBlock* current_block = mem_block_list;

    while (current_block) {
        if (current_block->offset == offset) {
            if (current_block->size >= size) {
                if (current_block->size > size) {
                    // Split the block
                    MemBlock* new_block = (MemBlock*)malloc(sizeof(MemBlock));
                    if (!new_block) return NULL;

                    new_block->offset = current_block->offset + size;
                    new_block->size = current_block->size - size;
                    new_block->is_block_free = 1;
                    new_block->next = current_block->next;

                    current_block->size = size;
                    current_block->next = new_block;
                }
                return ptr;
            } else {
                // Try to merge with next if possible
                if (current_block->next && current_block->next->is_block_free &&
                    current_block->size + current_block->next->size >= size) {
                    current_block->size += current_block->next->size;
                    MemBlock* next_block = current_block->next;
                    current_block->next = next_block->next;
                    free(next_block);

                    // Split again if oversized
                    if (current_block->size > size) {
                        MemBlock* new_block = (MemBlock*)malloc(sizeof(MemBlock));
                        if (!new_block) return NULL;

                        new_block->offset = current_block->offset + size;
                        new_block->size = current_block->size - size;
                        new_block->is_block_free = 1;
                        new_block->next = current_block->next;

                        current_block->size = size;
                        current_block->next = new_block;
                    }

                    return ptr;
                } else {
                    // Fallback: allocate new, copy data
                    void* new_ptr = mem_alloc(size);
                    if (new_ptr) {
                        memcpy(new_ptr, ptr, current_block->size);
                        mem_free(ptr);
                    }
                    return new_ptr;
                }
            }
        }

        current_block = current_block->next;
    }

    return NULL;
}

// Deinitialize memory pool, releasing all memory
void mem_deinit() {
    if (mem_pool) {
        free(mem_pool);
        mem_pool = NULL;
        mem_pool_size = 0;
    }

    MemBlock* current_block = mem_block_list;
    while (current_block) {
        MemBlock* next_block = current_block->next;
        free(current_block);
        current_block = next_block;
    }

    mem_block_list = NULL;
}
