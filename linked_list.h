#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include "memory_manager.h"

// Define the Node structure
typedef struct Node {
    uint16_t data;      // 16-bit unsigned integer data
    struct Node* next;  // Pointer to the next node
} Node;

// List initialization
void list_init(Node** head, size_t size);

// Insertion functions
void list_insert(Node** head, uint16_t data);
void list_insert_after(Node* prev_node, uint16_t data);
void list_insert_before(Node** head, Node* next_node, uint16_t data);

// Deletion function
void list_delete(Node** head, uint16_t data);

// Search function
Node* list_search(Node** head, uint16_t data);

// Display functions
void list_display(Node** head);
void list_display_range(Node** head, Node* start_node, Node* end_node);

// Count function
size_t list_current_nodes(Node* head);

// Cleanup function
void list_cleanup(Node** head);

#endif // LINKED_LIST_H
