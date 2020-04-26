#include "svc.h"
#include <stdio.h>
#include <stdlib.h>

struct node {
    int value;
    struct node *next;
    struct node *prev;
};

struct node* list_init(int value) {
    struct node *head = malloc(sizeof(struct node));
    head->value = value;
    head->next = head;
    head->prev = head;

    return head;
}

void list_add(struct node *head, int value) {
    struct node *n = malloc(sizeof(struct node));
    n->value = value;

    struct node *curr = head;

    while (curr->next != head) {
        curr = curr->next;
    }

    curr->next = n;
    n->next = head;
    n->prev = curr;
    head->prev = n; //update head as well
}

//We assume the node n exists in the head
void list_delete(struct node **head, struct node *n) {
    struct node *curr = *head;

    while (curr != n) {
        curr = curr->next;
    }

    curr->prev->next = curr->next;
    curr->next->prev = curr->prev;

    if (curr == *head) {
        *head = (*head)->next;
    }

    free(n);
}

struct node* list_next(const struct node *n) {
    if (n == NULL) {
        return NULL;
    }

    return n->next;
}

void list_free(struct node* head) {
    struct node *curr = head;

    do {
        struct node *temp = curr->next;
        free(curr);
        curr = temp;
    } while (curr != head);
}

int main() {
    struct node *head = list_init(1);
    list_add(head, 2);
    list_add(head, 3);
    list_add(head, 4);

    //Assumption: should print 1, 2, 3, 4
    struct node *cursor = head;
    do {
        printf("%d\n", cursor->value);
        cursor = list_next(cursor);
    } while (cursor != head);
    puts("");

    //Delete the first element
    list_delete(&head, head);

    //Assumption: should print 3, 4, 2
    cursor = head->next;
    do {
        printf("%d\n", cursor->value);
        cursor = list_next(cursor);
    } while (cursor != head->next);

    list_free(head);
}

#endif
