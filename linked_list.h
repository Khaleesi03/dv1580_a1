<<<<<<< HEAD
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>
#include <stdint.h>

typedef struct Node {
    uint16_t data;
    struct Node* next;
} Node;

void list_init(Node** head, size_t size);
void list_insert(Node** head, uint16_t data);
void list_insert_after(Node* prev_node, uint16_t data);
void list_insert_before(Node** head, Node* next_node, uint16_t data);
void list_delete(Node** head, uint16_t data);
Node* list_search(Node** head, uint16_t data);
void list_display(Node** head);
int list_count_nodes(Node** head);
void list_cleanup(Node** head);

#endif // LINKED_LIST_H
=======
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "memory_manager.h"

typedef struct Node {
    uint16_t data;
    struct Node* next;
} Node;

// Initialization function
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

// Nodes count function
int list_count_nodes(Node** head);

// Cleanup function
void list_cleanup(Node** head);

#endif // LINKED_LIST_H
>>>>>>> d8675c2f0fdc43be70019f38d9fe0d8fcc2aa578
