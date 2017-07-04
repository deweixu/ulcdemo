#include <stdio.h>
#include <stdlib.h>

typedef struct stack_node* stack_link;

struct stack_node {
    int item;
    stack_link next;
};

void STACK_init();
int STACK_empty();
void STACK_push(int);
int STACK_pop();