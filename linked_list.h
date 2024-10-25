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
