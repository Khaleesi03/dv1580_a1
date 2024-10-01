#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static void *memory_pool;
static size_t pool_size;

// Initialization function
void mem_init(size_t size)
{
    memory_pool = malloc(size); // Allocate the memory pool
    if (!memory_pool) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    pool_size = size;
    memset(memory_pool, 0, pool_size); // Initialize memory to zero
}

// Allocation function
void* mem_alloc(size_t size)
{
    if (size + sizeof(size_t) > pool_size) {
        return NULL; // Not enough space in the pool
    }

    void* current_block = memory_pool;
    while ((char*)current_block < (char*)memory_pool + pool_size) {
        size_t block_size = *(size_t*)current_block;

        if (block_size == 0) { // Free block
            if (pool_size - ((char*)current_block - (char*)memory_pool) >= size + sizeof(size_t)) {
                // Mark block as allocated
                *(size_t*)current_block = size;
                return (char*)current_block + sizeof(size_t);
            }
        }
        current_block = (char*)current_block + block_size + sizeof(size_t);
    }

    return NULL; // No free block found
}

// Deallocation function
void mem_free(void* block)
{
    if (block == NULL) {
        return;
    }

    // Set the size of the block to 0, indicating it's free
    size_t* block_size_ptr = (size_t*)((char*)block - sizeof(size_t));
    *block_size_ptr = 0;
}

// Resize function
void* mem_resize(void* block, size_t new_size)
{
    if (block == NULL) {
        return NULL;
    }

    // Get the original size of the block
    size_t original_size = *(size_t*)((char*)block - sizeof(size_t));

    // Allocate new memory for the block
    void* new_block = mem_alloc(new_size);

    // Copy the data from the old block to the new block
    if (new_block != NULL) {
        memcpy(new_block, block, original_size < new_size ? original_size : new_size);
    }

    // Deallocate the old block
    mem_free(block);

    return new_block;
}

// Node structure
typedef struct Node {
    char* key;
    int value;
    struct Node* next;
} Node;

// List initialization
void list_init(Node** head)
{
    *head = NULL;
}

// Insertion functions
void list_insert(Node** head, const char* key, int value)
{
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation for node failed.\n");
        return;
    }

    // Allocate memory for the key using custom mem_alloc
    new_node->key = (char*)mem_alloc(strlen(key) + 1);
    if (!new_node->key) {
        printf("Memory allocation for key failed.\n");
        mem_free(new_node);
        return;
    }

    strcpy(new_node->key, key); // Copy the key
    new_node->value = value;
    new_node->next = *head;
    *head = new_node;
}

// Deletion function
void list_delete(Node** head, const char* key)
{
    Node* current = *head;
    Node* previous = NULL;

    while (current != NULL && strcmp(current->key, key) != 0) {
        previous = current;
        current = current->next;
    }

    if (current != NULL) {
        if (previous != NULL) {
            previous->next = current->next;
        } else {
            *head = current->next;
        }

        // Free the key string using mem_free
        mem_free(current->key);

        // Free the current node using mem_free
        mem_free(current);
    }
}

// Search function
Node* list_search(Node* head, const char* key)
{
    Node* current = head;

    while (current != NULL && strcmp(current->key, key) != 0) {
        current = current->next;
    }

    return current;
}

// Display all nodes
void list_display(Node* head)
{
    Node* current = head;

    while (current != NULL) {
        printf("Key: %s, Value: %d\n", current->key, current->value);
        current = current->next;
    }
}

// Display a range of nodes
void list_display_range(Node* start, Node* end)
{
    Node* current = start;

    while (current != NULL && current != end) {
        printf("Key: %s, Value: %d\n", current->key, current->value);
        current = current->next;
    }
}

// Current nodes function
size_t list_current_nodes(Node* head)
{
    size_t count = 0;
    Node* current = head;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

// Cleanup function
void list_cleanup(Node** head)
{
    if (head == NULL || *head == NULL) {
        printf("List is already empty.\n");
        return;
    }

    Node* current = *head;
    while (current != NULL) {
        Node* next = current->next;

        // Free the key string using mem_free
        mem_free(current->key);

        // Free the current node using mem_free
        mem_free(current);

        current = next;
    }

    *head = NULL;  // Set the head pointer to NULL after cleanup
}