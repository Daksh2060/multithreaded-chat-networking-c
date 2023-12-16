#ifndef _LIST_H_
#define _LIST_H_

#include <stdbool.h>

#define LIST_SUCCESS 0
#define LIST_FAIL -1

// Node structure representing an element in the linked list
typedef struct Node_s {
    void *pItem;             // Pointer to the data item
    struct Node_s *pNext;    // Pointer to the next node in the list
    struct Node_s *pPrev;    // Pointer to the previous node in the list
} Node;

// Enum for out-of-bounds conditions
enum ListOutOfBounds {
    LIST_OOB_START,    // Operation went beyond the start of the list
    LIST_OOB_END       // Operation went beyond the end of the list
};

// Linked list structure
typedef struct List_s {
    Node *pFirstNode;               // Pointer to the first node in the list
    Node *pLastNode;                // Pointer to the last node in the list
    Node *pCurrentNode;             // Pointer to the current node for iteration
    int count;                      // Number of nodes in the list
    struct List_s *pNextFreeHead;   // Pointer to the next free head in the list pool
    enum ListOutOfBounds lastOutOfBoundsReason;  // Last out-of-bounds reason for error reporting
} List;

#define LIST_MAX_NUM_HEADS 100
#define LIST_MAX_NUM_NODES 1000

// Function to create a new linked list
List *List_create();

// Function to get the count of nodes in the list
int List_count(List *pList);

// Functions for accessing data in the list
void *List_first(List *pList);
void *List_last(List *pList);
void *List_next(List *pList);
void *List_prev(List *pList);
void *List_curr(List *pList);

// Functions for inserting and appending nodes to the list
int List_insert_after(List *pList, void *pItem);
int List_insert_before(List *pList, void *pItem);
int List_append(List *pList, void *pItem);
int List_prepend(List *pList, void *pItem);

// Function to remove the current node from the list
void *List_remove(List *pList);

// Function to concatenate two lists
void List_concat(List *pList1, List *pList2);

// Function to free the memory occupied by the list and its nodes
typedef void (*FREE_FN)(void *pItem);
void List_free(List *pList, FREE_FN pItemFreeFn);

// Function to remove and return the last node in the list
void *List_trim(List *pList);

// Function to search for a specific item in the list
typedef bool (*COMPARATOR_FN)(void *pItem, void *pComparisonArg);
void *List_search(List *pList, COMPARATOR_FN pComparator, void *pComparisonArg);

#endif // _LIST_H_

