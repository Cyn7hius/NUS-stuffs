/*************************************
* Lab 1 Exercise 2
* Name: Tan Wei Xiang, Calvin
* Student No: A0217529
* Lab Group: 12
*************************************/
/*************************************
* Lab 1 Exercise 2
* Name: Dillon Tan Kiat Wee
* Student No: A0218033R
* Lab Group: 13
*************************************/


#include <stdlib.h>
#include <stdio.h>
#include "node.h"


// Add in your implementation below to the respective functions
// Feel free to add any headers you deem fit 


// Inserts a new node with data value at index (counting from head
// starting at 0).
// Note: index is guaranteed to be valid.
void insert_node_at(list *lst, int index, int data) {

    node *currNode = lst->head;

    node *newNode = malloc(sizeof(node));
    newNode->data = data;

    // Case where lst is empty
    if (currNode == NULL) {
        lst->head = newNode;
        return;
    }

    node *nextNode = currNode->next;

    if (index == 0) {
        lst->head = newNode;
        newNode->next = currNode;
        return;
    }

    for (int i = 0; i < index - 1; i++) {
        currNode = nextNode;
        nextNode = currNode->next;
    }

    currNode->next = newNode;
    newNode->next = nextNode;
    return;

}

// Deletes node at index (counting from head starting from 0).
// Note: index is guarenteed to be valid.
void delete_node_at(list *lst, int index) {

    node *currNode = lst->head;
    node *nextNode = currNode->next;

    // might be redundant
    if (index == 0) {
       lst->head = nextNode;
       return; 
    }

    for (int i = 0; i < index - 1; i++) {
        currNode = nextNode;
        nextNode = currNode->next;
    }
    currNode->next = nextNode->next;
    free(nextNode); // delete the node
    return;

}

// Search list by the given element.
// If element not present, return -1 else return the index. If lst is empty return -2.
//Printing of the index is already handled in ex2.c
int search_list(list *lst, int element) {
    
    if (lst == NULL) {
        return -2;
    }

    node *currNode = lst->head;
    int index = 0;
    while(currNode != NULL) {
        if (currNode->data == element) {
            return index;
        }
        currNode = currNode->next;
        index++;
    }

    return -1;

}

// Reverses the list with the last node becoming the first node.
void reverse_list(list *lst) {
    
    node *currNode = lst->head;
    node *prevNode = NULL;
    node *nextNode = NULL;

    while (currNode != NULL) {
        nextNode = currNode->next;
        currNode->next = prevNode;
        prevNode = currNode;
        currNode = nextNode;
    }
    lst->head = prevNode;


}

// Resets list to an empty state (no nodes) and frees
// any allocated memory in the process
void reset_list(list *lst) {
    node *currNode;
    node *nextNode = lst->head;

    while (nextNode != NULL) {
        currNode = nextNode;
        nextNode = nextNode->next;
        free(currNode);
    }

    lst->head = NULL;

}
