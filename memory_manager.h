#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>

// Initializes the memory manager with a specified size of memory pool
void mem_init(size_t size);

// Allocates a block of memory of the specified size
void* mem_alloc(size_t size);

// Frees the specified block of memory
void mem_free(void* block);

// Resizes an allocated block to the new size, returning the new block
void* mem_resize(void* block, size_t new_size);

// Frees up the memory pool allocated by mem_init
void mem_deinit();

#endif // MEMORY_MANAGER_H
<<<<<<< HEAD
=======
=======
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


int mem_init(size_t size);
void* mem_alloc(size_t size);
void mem_free(void* block);
void* mem_resize(void* block, size_t size);
void mem_deinit();

#endif // MEMORY_MANAGER_H
>>>>>>> d8675c2f0fdc43be70019f38d9fe0d8fcc2aa578
>>>>>>> 1626eba23f0a1762b13dfec45f83fd62295d2eeb
