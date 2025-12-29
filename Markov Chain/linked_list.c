#include "linked_list.h"

// linked_list.c
// -------------
// Implementation of a minimal singly-linked list.
//
// This list is used by the Markov chain as a simple container for all words
// (MarkovNode objects). The list does not own/copy the content of "data";
// it only stores the pointer that is passed to it.

int add(LinkedList *link_list, void *data)
{
    // Allocate a new list node.
    Node *new_node = malloc(sizeof(Node));
    if (new_node == NULL)
    {
        // Allocation failed.
        return 1;
    }

    // Initialize the node with the given data pointer.
    // Note: Node::data is MarkovNode*, but the function keeps a (void*) API.
    *new_node = (Node){data, NULL};

    // Insert into an empty list.
    if (link_list->first == NULL)
    {
        link_list->first = new_node;
        link_list->last = new_node;
    }
    else
    {
        // Append to tail in O(1).
        link_list->last->next = new_node;
        link_list->last = new_node;
    }

    // Maintain list size for O(1) random selection later.
    link_list->size++;
    return 0;
}
