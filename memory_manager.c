#include <stdlib.h>
#include <stddef.h>

static void *memory_pool = NULL;
static size_t memory_pool_size = 0;

void mem_init(size_t size) {
    if (memory_pool != NULL) {
        free(memory_pool);
    }
    memory_pool = malloc(size);
    if (memory_pool != NULL) {
        memory_pool_size = size;
    } else {
        memory_pool_size = 0;
    }
}

void *mem_alloc(size_t size) {
    if (memory_pool == NULL || size == 0 || size > memory_pool_size) {
        return NULL;
    }

    // Simple first-fit allocation strategy
    unsigned char *ptr = (unsigned char *)memory_pool;
    size_t offset = 0;

    while (offset + size <= memory_pool_size) {
        size_t *block_size = (size_t *)(ptr + offset);
        if (*block_size == 0) {
            *block_size = size;
            return (void *)(ptr + offset + sizeof(size_t));
        }
        offset += *block_size + sizeof(size_t);
    }

    return NULL; // No suitable block found
}
void mem_free(void *block) {
    if (block == NULL || memory_pool == NULL) {
        return;
    }

    unsigned char *ptr = (unsigned char *)block;
    size_t *block_size = (size_t *)(ptr - sizeof(size_t));

    if ((unsigned char *)block_size >= (unsigned char *)memory_pool &&
        (unsigned char *)block_size < (unsigned char *)memory_pool + memory_pool_size) {
        *block_size = 0; // Mark the block as free
    }
}

void *mem_resize(void *block, size_t size) {
    if (block == NULL) {
        return mem_alloc(size);
    }

    unsigned char *ptr = (unsigned char *)block;
    size_t *block_size = (size_t *)(ptr - sizeof(size_t));

    if ((unsigned char *)block_size < (unsigned char *)memory_pool ||
        (unsigned char *)block_size >= (unsigned char *)memory_pool + memory_pool_size) {
        return NULL; // Block is out of bounds
    }

    size_t current_size = *block_size;
    if (size <= current_size) {
        return block; // No need to resize
    }

    void *new_block = mem_alloc(size);
    if (new_block != NULL) {
        memcpy(new_block, block, current_size);
        mem_free(block);
    }

    return new_block;
}

void mem_deinit() {
    if (memory_pool != NULL) {
        free(memory_pool);
        memory_pool = NULL;
        memory_pool_size = 0;
    }
}
