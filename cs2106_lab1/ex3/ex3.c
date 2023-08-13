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


#include <stdio.h>
#include <stdlib.h>

#include "function_pointers.h"
#include "node.h"

// The runner is empty now! Modify it to fulfill the requirements of the
// exercise. You can use ex2.c as a template

// DO NOT initialize the func_list array in this file. All initialization
// logic for func_list should go into function_pointers.c.

// Macros
#define SUM_LIST 0
#define INSERT_AT 1
#define DELETE_AT 2
#define SEARCH_LIST 3
#define REVERSE_LIST 4
#define RESET_LIST 5
#define LIST_LEN 6
#define MAP 7

void run_instruction(FILE *inputFile, list *lst, int instr);
void print_list(list *lst);
void print_index(int index);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: expecting 1 argument, %d found\n", argc - 1);
        exit(1);
    }

    // We read in the file name provided as argument
    FILE *inputFile;
    inputFile = fopen(argv[1], "r"); 
    if (inputFile == NULL) {
        exit(1);
    }

    // Update the array of function pointers
    // DO NOT REMOVE THIS CALL
    // (You may leave the function empty if you do not need it)
    update_functions();

    
    // Rest of code logic here
    list *lst = (list *)malloc(sizeof(list));
    lst->head = NULL;


    int instr;
    while (fscanf(inputFile, "%d", &instr) == 1) {
        run_instruction(inputFile, lst, instr);
    }
    
    fclose(inputFile);

    reset_list(lst);
    free(lst);

    exit(0);
    
}

// Takes an instruction enum and runs the corresponding function
// We assume input always has the right format (no input validation on runner)
void run_instruction(FILE *inputFile, list *lst, int instr) {
    int index, data, element;
    switch (instr) {
        case SUM_LIST:
            ; // to bypass error: a label can only be part of a statement and a declaration is not a statement
            //https://stackoverflow.com/questions/46341364/label-can-only-be-used-as-part-of-a-statement-error
            long sum = sum_list(lst);
            printf("%ld\n", sum);
            break;
        case INSERT_AT:
            fscanf(inputFile, "%d %d", &index, &data);
            insert_node_at(lst, index, data);
            break;
        case DELETE_AT:
            fscanf(inputFile, "%d", &index);
            delete_node_at(lst, index);
            break;
        case SEARCH_LIST:
            fscanf(inputFile, "%d", &element);
            int ind = search_list(lst, element);
            print_index(ind);
            break;
        case REVERSE_LIST:
            reverse_list(lst);
            break;
        case RESET_LIST:
            reset_list(lst);
            break;
        case LIST_LEN:
            ;
            int len = list_len(lst);
            printf("%d\n", len);
            break;
        case MAP:
            fscanf(inputFile, "%d", &index);
            map(lst, func_list[index]);
            break;
    }
}


// From ex2
// Prints out the whole list in a single line
void print_list(list *lst) {
    if (lst->head == NULL) {
        printf("[ ]\n");
        return;
    }

    printf("[ ");
    node *curr = lst->head;
    do {
        printf("%d ", curr->data);
        curr = curr->next;
    } while (curr != NULL);
    printf("]\n");
}

//Print index
void print_index(int index)
{
    if(index == -2)
        printf("{}\n");
    else
        printf("{%d}\n", index);
}

