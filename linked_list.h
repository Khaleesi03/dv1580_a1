#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>

// Define the node structure
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Function to create a new node
Node* create_node(int data);

// Function to insert a node at the beginning
void insert_at_beginning(Node** head, int data);

// Function to insert a node at the end
void insert_at_end(Node** head, int data);

// Function to delete a node
void delete_node(Node** head, int key);

// Function to search for a node
Node* search_node(Node* head, int key);

// Function to print the linked list
void print_list(Node* head);

#endif // LINKED_LIST_H