/*************************************
* Lab 1 Exercise 3
* Name: Tan Wei Xiang, Calvin
* Student No: A0217529
* Lab Group: 12
*************************************/
/*************************************
* Lab 1 Exercise 3
* Name: Dillon Tan Kiat Wee
* Student No: A0218033R
* Lab Group: 13
*************************************/


#include "node.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Add in your implementation below to the respective functions
// Feel free to add any headers you deem fit (although you do not need to)


// Traverses list and returns the sum of the data values
// of every node in the list.
long sum_list(list *lst) {
    long sum = 0;
    node *currNode = lst->head;
    while (currNode != NULL) {
        sum += currNode->data;
        currNode = currNode->next;
    }
    return sum;

}

// Traverses list and returns the number of data values in the list.
int list_len(list *lst) {
    long len = 0;
    node *currNode = lst->head;
    while (currNode != NULL) {
        len++;
        currNode = currNode->next;
    }
    return len;

}



// Inserts a new node with data value at index (counting from head
// starting at 0).
// Note: index is guaranteed to be valid.
void insert_node_at(list *lst, int index, int data) {

    node *currNode = lst->head;
    node *newNode = (node *)malloc(sizeof(node));
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



// Traverses list and applies func on data values of
// all elements in the list.
void map(list *lst, int (*func)(int)) {
    node *currNode = lst->head;
    while (currNode != NULL) {
        currNode->data = (*func)(currNode->data);
        currNode = currNode->next;
    }

}
	

