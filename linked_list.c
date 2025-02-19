#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Node {
    uint16_t data;
    struct Node* next;
} Node;

// Custom memory manager functions
void* mem_alloc(size_t size);
void mem_free(void* ptr);

// Initialization function
void list_init(Node** head, size_t size) {
    *head = NULL;
}

// Insertion functions
void list_insert(Node** head, uint16_t data) {
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
}

void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Previous node cannot be NULL\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (next_node == NULL) {
        printf("Next node cannot be NULL\n");
        return;
    }

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        return;
    }
    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
    } else {
        Node* temp = *head;
        while (temp != NULL && temp->next != next_node) {
            temp = temp->next;
        }
        if (temp == NULL) {
            printf("Next node not found in the list\n");
            mem_free(new_node);
            return;
        }
        new_node->next = next_node;
        temp->next = new_node;
    }
}

// Deletion function
void list_delete(Node** head, uint16_t data) {
    if (*head == NULL) {
        printf("List is empty\n");
        return;
    }

    Node* temp = *head;
    Node* prev = NULL;

    if (temp != NULL && temp->data == data) {
        *head = temp->next;
        mem_free(temp);
        return;
    }

    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("Data not found in the list\n");
        return;
    }

    prev->next = temp->next;
    mem_free(temp);
}

// Search function
Node* list_search(Node** head, uint16_t data) {
    Node* temp = *head;
    while (temp != NULL) {
        if (temp->data == data) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// Display functions
void list_display(Node** head) {
    Node* temp = *head;
    printf("[");
    while (temp != NULL) {
        printf("%d", temp->data);
        temp = temp->next;
        if (temp != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}

void list_display_range(Node** head, Node* start_node, Node* end_node) {
    Node* temp = *head;
    int print = (start_node == NULL);

    printf("[");
    while (temp != NULL) {
        if (temp == start_node) {
            print = 1;
        }
        if (print) {
            printf("%d", temp->data);
            if (temp != end_node) {
                printf(", ");
            }
        }
        if (temp == end_node) {
            break;
        }
        temp = temp->next;
    }
    printf("]\n");
}

// Nodes count function
int list_count_nodes(Node** head) {
    int count = 0;
    Node* temp = *head;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

// Cleanup function
void list_cleanup(Node** head) {
    Node* temp = *head;
    while (temp != NULL) {
        Node* next = temp->next;
        mem_free(temp);
        temp = next;
    }
    *head = NULL;
}
