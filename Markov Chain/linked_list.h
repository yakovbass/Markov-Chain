// linked_list.h
// ----------------
// A tiny singly-linked list used as the "database" container for the Markov
// chain.
//
// The list nodes store pointers to MarkovNode objects (defined in
// markov_chain.h). We forward-declare MarkovNode here to avoid circular
// includes.

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <stdlib.h> // malloc(), free()

typedef struct Node {
    struct MarkovNode *data;
    struct Node *next;
} Node;

typedef struct LinkedList {
    Node *first;
    Node *last;
    int size;
} LinkedList;

/**
 * Append a new node to the end of the given linked list.
 *
 * Important:
 * - "data" is expected to point to a dynamically allocated MarkovNode.
 * - This function does not copy the data; it only stores the pointer.
 * @param link_list Link list to add data to
 * @param data pointer to dynamically allocated data (MarkovNode*)
 * @return 0 on success, 1 otherwise
 */
int add(LinkedList *link_list, void *data);

#endif // _LINKEDLIST_H_
