#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Initialization function
// Sets up the memory pool of the specified size
void mem_init(size_t size);

// Allocation function
// Allocates a block of memory of the specified size
void* mem_alloc(size_t size);

// Deallocation function
// Frees a block of memory, marking it as available
void mem_free(void* block);

// Resize function
// Resizes an allocated block to the new size, returning the new block
void* mem_resize(void* block, size_t new_size);

#endif // MEMORY_MANAGER_H
