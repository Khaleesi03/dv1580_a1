#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "memory_manager.h"

typedef struct Node {
    uint16_t data;            // Stores the data as an unsigned 16-bit integer
    struct Node* next;       // Pointer to the next node in the list
} Node;

// Initialization function
void list_init(Node** head, size_t size) {
    mem_init(size);
    *head = NULL;
}

// Insertion function at the rear
void list_insert(Node** head, uint16_t data) {
    if (head == NULL) return;

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed. Cannot insert node.\n");
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// Insertion function after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Previous node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed. Cannot insert node.\n");
        return;
    }
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

// Insertion function before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (head == NULL) return;
    if (next_node == NULL) {
        printf("Next node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed. Cannot insert node.\n");
        return;
    }
    new_node->data = data;

    // If next_node is head, insert at head
    if (*head == next_node) {
        new_node->next = next_node;
        *head = new_node;
        return;
    }

    // Find previous node
    Node* current = *head;
    while (current != NULL && current->next != next_node) {
        current = current->next;
    }

    if (current == NULL) {
        printf("Next node not found in the list.\n");
        mem_free(new_node);
        return;
    }

    new_node->next = next_node;
    current->next = new_node;
}

// Deletion function
void list_delete(Node** head, uint16_t data) {
    if (head == NULL || *head == NULL) {
        printf("List is empty. Cannot delete.\n");
        return;
    }

    Node* current = *head;
    Node* prev = NULL;

    // If head node holds the data to be deleted
    if (current->data == data) {
        *head = current->next;
        mem_free(current);
        return;
    }

    // Search for the node to be deleted
    while (current != NULL && current->data != data) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Node with data %u not found.\n", data);
        return;
    }

    prev->next = current->next;
    mem_free(current);
}

// Search function
Node* list_search(Node** head, uint16_t data) {
    if (head == NULL) return NULL;

    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Display function
void list_display(Node** head) {
    if (head == NULL) {
        printf("[]\n");
        return;
    }

    Node* current = *head;
    printf("[");
    while (current != NULL) {
        printf("%u", current->data);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

// Display range function
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    if (head == NULL || *head == NULL) {
        printf("[]\n");
        return;
    }

    Node* current = (start_node == NULL) ? *head : start_node;
    printf("[");
    while (current != NULL) {
        printf("%u", current->data);
        if (current == end_node) {
            break;
        }
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]\n");
}

// Nodes count function
int list_count_nodes(Node** head) {
    if (head == NULL) return 0;

    int count = 0;
    Node* current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

// Cleanup function
void list_cleanup(Node** head) {
    if (head == NULL) return;

    Node* current = *head;
    while (current != NULL) {
        Node* next = current->next;
        mem_free(current);
        current = next;
    }
    *head = NULL;
    mem_deinit();
}
