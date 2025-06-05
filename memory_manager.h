#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>

// Initializes the memory manager with a specified size of memory pool
int mem_init(size_t size);

// Allocates a block of memory of the specified size
void* mem_alloc(size_t size);

// Frees the specified block of memory
void mem_free(void* block);

// Resizes an allocated block to the new size, returning the new block
void* mem_resize(void* block, size_t new_size);

// Frees up the memory pool allocated by mem_init
void mem_deinit();

#endif // MEMORY_MANAGER_H
