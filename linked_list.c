#include <stdio.h>
#include "linked_list.h"
#include "memory_manager.h"

void list_init(Node **head, size_t size) {
    #ifdef DEBUG
    printf("list_init: ensuring memory manager is reset\n");
    #endif

    mem_deinit();  // Ensure clean state for memory manager

    if (mem_init(size) != 0) {
        fprintf(stderr, "Memory manager initialization failed.\n");
        return;
    }

    *head = NULL;
}

void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        fprintf(stderr, "Memory allocation failed in list_insert\n");
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* temp = *head;
        while (temp->next)
            temp = temp->next;
        temp->next = new_node;
    }
}

void list_insert_after(Node* prev_node, uint16_t data) {
    if (!prev_node) return;

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        fprintf(stderr, "Memory allocation failed in list_insert_after\n");
        return;
    }
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (!head || !next_node) return;

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        fprintf(stderr, "Memory allocation failed in list_insert_before\n");
        return;
    }

    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    Node* prev = *head;
    while (prev && prev->next != next_node)
        prev = prev->next;

    if (!prev) {
        mem_free(new_node);
        fprintf(stderr, "Next node not found in list_insert_before\n");
        return;
    }

    new_node->next = next_node;
    prev->next = new_node;
}

void list_delete(Node** head, uint16_t data) {
    if (!head || !*head) return;

    Node* temp = *head;
    Node* prev = NULL;

    while (temp && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    if (!temp) return;

    if (!prev)
        *head = temp->next;
    else
        prev->next = temp->next;

    mem_free(temp);
}

Node* list_search(Node** head, uint16_t data) {
    Node* current = *head;
    while (current) {
        if (current->data == data)
            return current;
        current = current->next;
    }
    return NULL;
}

void list_display(Node** head) {
    Node* current = *head;

    printf("[");
    int first = 0;  // Flag to avoid printing comma before the first element

    while (current != NULL) {
        if (!first) {
            printf(", ");
        }
        printf("%d", current->data);
        first = 1;
        current = current->next;
    }
    printf("]\n");
}

void list_display_range(Node** head, Node* start_node, Node* end_node) {
    Node* current = *head;
    int print = (start_node == NULL);  // if start_node == NULL, start printing immediately
    int first = 1; // track if it's the first element printed

    printf("[");
    while (current) {
        if (current == start_node)
            print = 1;
        if (print) {
            if (!first) {
                printf(", ");
            }
            printf("%d", current->data);
            first = 0;

            if (current == end_node)
                break;
        }
        current = current->next;
    }
    printf("]");
}


int list_count_nodes(Node** head) {
    int count = 0;
    Node* current = *head;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

void list_cleanup(Node** head) {
    Node* current = *head;
    while (current) {
        Node* temp = current;
        current = current->next;
        mem_free(temp);
    }
    *head = NULL;
}
