#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "memory_manager.h"
#include "linked_list.h"

typedef struct Node {
    uint16_t data;            // Stores the data as an unsigned 16-bit integer
    struct Node* next;       // Pointer to the next node in the list
} Node;

// Initialization function
void list_init(Node** head, size_t size) {
    *head = NULL; // Set the head to NULL to indicate an empty list
}

// Insertion function at the rear
void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node)); // Allocate a new node
    if (!new_node) {
        printf("Memory allocation failed!\n");
        return;
    }

    new_node->data = data; // Set the data for the new node
    new_node->next = NULL; // Initialize the next pointer to NULL

    if (*head == NULL) {
        *head = new_node; // If the list is empty, new node becomes the head
        return;
    }

    Node* temp = *head; // Traverse to the end of the list
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = new_node; // Link the new node at the end
}

// Insertion function after a given node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("The previous node cannot be NULL.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed!\n");
        return;
    }

    new_node->data = data;
    new_node->next = prev_node->next; // Link the new node with the next node
    prev_node->next = new_node; // Link the previous node to the new node
}

// Insertion function before a given node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (*head == NULL || next_node == NULL) {
        printf("List is empty or the next node is NULL.\n");
        return;
    }

    if (*head == next_node) { // Inserting before the head
        Node* new_node = (Node*)mem_alloc(sizeof(Node));
        if (!new_node) {
            printf("Memory allocation failed!\n");
            return;
        }
        new_node->data = data;
        new_node->next = *head; // Link to the current head
        *head = new_node; // Update head to the new node
        return;
    }

    Node* current = *head;
    while (current != NULL && current->next != next_node) {
        current = current->next; // Traverse to find the node before next_node
    }

    if (current == NULL) {
        printf("The next node is not found in the list.\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed!\n");
        return;
    }

    new_node->data = data;
    new_node->next = next_node; // Link new node to next_node
    current->next = new_node; // Link the previous node to the new node
}

// Deletion function
void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        printf("List is empty. Cannot delete.\n");
        return;
    }

    Node* temp = *head;
    Node* prev = NULL;

    // Check if head node holds the data
    if (temp != NULL && temp->data == data) {
        *head = temp->next; // Change head
        mem_free(temp); // Free the old head
        return;
    }

    // Search for the data to be deleted
    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    // If data was not present in the list
    if (temp == NULL) {
        printf("Data not found in the list.\n");
        return;
    }

    prev->next = temp->next; // Unlink the node from linked list
    mem_free(temp); // Free the memory
}

// Search function
Node* list_search(Node** head, uint16_t data) {
    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            return current; // Return the node if found
        }
        current = current->next;
    }
    return NULL; // Data not found
}

// Display function
void list_display(Node** head) {
    Node* current = *head;
    printf("[");
    while (current != NULL) {
        printf("%d", current->data);
        current = current->next;
        if (current != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Nodes count function
int list_count_nodes(Node** head) {
    int count = 0;
    Node* current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count; // Return the count of nodes
}

// Cleanup function
void list_cleanup(Node** head) {
    Node* current = *head;
    Node* next;

    while (current != NULL) {
        next = current->next; // Store next node
        mem_free(current); // Free current node
        current = next; // Move to next node
    }
    *head = NULL; // Update head to NULL
}
