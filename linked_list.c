#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>

void list_init(Node** head, size_t size) {
    *head = NULL;         // Ensure the head pointer is set to NULL (empty list)
    (void)size;           // Explicit cast to void to suppress unused warning
}

//
// Inserts a new node with given data at the end of the list
//
void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)malloc(sizeof(Node));  // Dynamically allocate new node
    if (!new_node) {
        fprintf(stderr, "list_insert: allocation failed\n");
        return;
    }

    new_node->data = data;    // Assign data to new node
    new_node->next = NULL;    // New node will be the last node, so next is NULL

    if (!*head) {
        // If list is empty, new node becomes the head
        *head = new_node;
        return;
    }

    // Traverse to the end of the list
    Node* current = *head;
    while (current->next) {
        current = current->next;
    }

    current->next = new_node;  // Append the new node at the end
}

//
// Inserts a new node with given data after the specified node
//
void list_insert_after(Node* prev_node, uint16_t data) {
    if (!prev_node) {
        fprintf(stderr, "list_insert_after: prev_node is NULL\n");
        return;
    }

    Node* new_node = (Node*)malloc(sizeof(Node));  // Allocate new node
    if (!new_node) {
        fprintf(stderr, "list_insert_after: allocation failed\n");
        return;
    }

    new_node->data = data;                 // Set data
    new_node->next = prev_node->next;      // Link new node to the node after prev_node
    prev_node->next = new_node;            // Insert new node into the list after prev_node
}

//
// Inserts a new node with given data before a specified node (next_node)
// - If next_node is the head, insert new node at the beginning
//
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (!next_node) {
        fprintf(stderr, "list_insert_before: next_node is NULL\n");
        return;
    }

    // If next_node is the head, insert at front
    if (*head == next_node) {
        Node* new_node = (Node*)malloc(sizeof(Node));
        if (!new_node) {
            fprintf(stderr, "list_insert_before: allocation failed\n");
            return;
        }

        new_node->data = data;
        new_node->next = *head;
        *head = new_node;
        return;
    }

    // Find the node before next_node
    Node* current = *head;
    while (current && current->next != next_node) {
        current = current->next;
    }

    if (!current) {
        fprintf(stderr, "list_insert_before: next_node not found in list\n");
        return;
    }

    // Use insert_after logic to insert before target
    list_insert_after(current, data);
}

//
// Deletes the first node that contains the specified data value
//
void list_delete(Node** head, uint16_t data) {
    Node* current = *head;
    Node* prev = NULL;

    // Search for the node containing the data
    while (current && current->data != data) {
        prev = current;
        current = current->next;
    }

    if (!current) {
        // Reached end of list and data not found
        fprintf(stderr, "list_delete: value %u not found\n", data);
        return;
    }

    if (!prev) {
        // Node to be deleted is the head
        *head = current->next;
    } else {
        // Bypass current node
        prev->next = current->next;
    }

    free(current);  // Free memory used by node
}

//
// Searches for the first node containing the specified data
//
Node* list_search(Node** head, uint16_t data) {
    Node* current = *head;

    // Traverse list until data is found
    while (current) {
        if (current->data == data) {
            return current;  // Found matching node
        }
        current = current->next;
    }

    return NULL;  // Not found
}

//
// Displays all elements in the list as [val1, val2, ..., valN]
//
void list_display(Node** head) {
    Node* current = *head;
    printf("[");

    // Traverse and print node data
    while (current) {
        printf("%u", current->data);
        if (current->next) {
            printf(", ");  // Print separator if not last
        }
        current = current->next;
    }

    printf("]");
}

//
// Displays the contents of a subrange of the list from start_node to end_node (inclusive)
// - If start_node is NULL, starts from head
//
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    if (!*head) {
        // Empty list
        printf("[]");
        return;
    }

    if (!start_node) {
        // If start_node not specified, begin from head
        start_node = *head;
    }

    Node* current = start_node;
    printf("[");
    int first = 1;

    // Traverse from start_node to end_node
    while (current) {
        if (!first) {
            printf(", ");
        }
        printf("%u", current->data);
        if (current == end_node) {
            break;  // Stop at end_node (inclusive)
        }
        current = current->next;
        first = 0;
    }

    printf("]");
}

//
// Counts and returns the number of nodes in the list
//
int list_count_nodes(Node** head) {
    int count = 0;
    Node* current = *head;

    // Count nodes one-by-one
    while (current) {
        count++;
        current = current->next;
    }

    return count;
}

//
// Frees all nodes in the list and resets the head pointer to NULL
//
void list_cleanup(Node** head) {
    Node* current = *head;

    // Traverse and free each node
    while (current) {
        Node* temp = current;    // Store current pointer for deallocation
        current = current->next; // Move to next node
        free(temp);              // Free the current node
    }

    *head = NULL;  // Reset head pointer to signify an empty list
}
