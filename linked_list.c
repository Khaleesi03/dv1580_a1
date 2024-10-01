#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

//Define the Node structure
typedef struct Node {
    uint16_t data;      // 16-bit unsigned integer data
    struct Node* next;  // Pointer to the next node
} Node;

// List initialization
//Initialises the head of the list to NUll and sets up the memory pool using mem_init
void list_init(Node** head, size_t size) {
    *head = NULL; // Start with an empty list
    // Custom memory manager should handle the memory pool initialization
    mem_init(size); //INitialise memory manager with the given size
}

// Insertion functions
//Inserts a new node at the end of the list
//Inserts a new node with the given data at the rear end of the linked list
void list_insert(Node** head, uint16_t data) {
    //Allocate memory fo rhte new node using memory manager
    Node* new_node = (Node*) mem_alloc(sizeof(Node)); 
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    //If the lsit is empty, make the new node the head
    if (*head == NULL) {
        *head = new_node;
    } else {
        //Traverse to the end of the list and add the new node
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
}

//Insert after a specific node
//Inserts a new node with the given data immediately after the specified node
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("The previous node cannot be NULL\n");
        return;
    }
    //Allocate memory for hte new node using memory manager
    Node* new_node = (Node*) mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }
    //Insert the new ode after the previous node
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

// Insert before a specific node
// Inserts a new node with the given data immediately before the specified node
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (next_node == NULL || *head == NULL) {
        printf("The node to insert before cannot be NULL\n");
        return;
    }
    //Allocate memory for the new node using memory manager
    Node* new_node = (Node*) mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    new_node->data = data;

    //If the new node is the head, insert the new node as the new head
    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
    } else {
        //Traverse to find the node before the next_node
        Node* temp = *head;
        while (temp->next != next_node) {
            temp = temp->next;
        }
        //Insert the new node before next_node
        new_node->next = next_node;
        temp->next = new_node;
    }
}

// Deletion function
//Delete a node by value
//Deletes the first node with the given list
void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        printf("List is empty, cannot delete\n");
        return;
    }

    Node* temp = *head;
    Node* prev = NULL;
    
    //check if the node to delete is the head
    if (temp != NULL && temp->data == data) {
        *head = temp->next; //Move the head to the next node
        mem_free(temp);  // Use memory manager to free the memory of the old head
        return;
    }
    //Traverse the list to find the node with the matching data
    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }
    //If no node is found, print the message
    if (temp == NULL) {
        printf("Node with data %d not found\n", data);
        return;
    }
    //Unlink the node and free its memory
    prev->next = temp->next;
    mem_free(temp);  // Use custom memory manager
}


// Search function
//Search for a node by value
//Searches for the first node with the given data and returns a pointer to it
Node* list_search(Node** head, uint16_t data) {
    Node* temp = *head;
    //Traverse th elsit and check each node's data
    while (temp != NULL) {
        if (temp->data == data) {
            return temp; //Return the node if found
        }
        temp = temp->next;
    }
    return NULL; //Return null if the node is not found
}


// Display all nodes
//Display all nodes in the list
//Prints the entire linked list in the format [data1, data2, ...]
void list_display(Node** head) {
    Node* temp = *head;
    printf("[");
    while (temp != NULL) {
        printf("%d", temp->data);
        if (temp->next != NULL) {
            printf(", ");
        }
        temp = temp->next;
    }
    printf("]\n");
}

// Display a range of nodes
//Prints all nodes between start_node adn end_node inclusively
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    Node* temp = *head;

    // Start from the head if start_node is NULL
    if (start_node == NULL) {
        start_node = *head;
    }
    //Traverse to the starting node
    while (temp != start_node && temp != NULL) {
        temp = temp->next;
    }

    printf("[");
    while (temp != NULL) {
        printf("%d", temp->data);
        if (temp->next != end_node->next) {
            printf(", ");
        }
        temp = temp->next;

        // Stop if we reach the end node
        if (temp == end_node) {
            printf(", %d", temp->data);
            break;
        }
    }
    printf("]\n");
}

// Current nodes function
//Count the number of nodes in the list
//Returns the total number of nodes in the linked list
size_t list_current_nodes(Node* head)
{
    size_t count = 0;
    Node* current = head;

    //Traverse the list and increment the count for each node
    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

// Cleanup function
//Cleanup the list and free all nodes
//Frees all the nodes in the list to avoid memory leaks
void list_cleanup(Node** head) {
    Node* temp = *head;
    Node* next_node;

    // Traverse the list and free each node's memory
    while (temp != NULL) {
        next_node = temp->next;
        mem_free(temp);  // Use memory manager to free memory
        temp = next_node;
    }

    *head = NULL;  // Reset head to NULL after cleanup
}
