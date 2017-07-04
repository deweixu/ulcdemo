#include "stack.h"

static stack_link head;

stack_link NEW(int item, stack_link next)
{
    stack_link x = malloc(sizeof *x);
    x->item = item;
    x->next = next;
    return x;
}

void STACK_init()
{
    head = NULL;
}

int STACK_empty()
{
    return head == NULL;
}

void STACK_push(int item)
{
    head = NEW(item, head);
}

int STACK_pop()
{
    int item = head->item;
    stack_link t = head->next;
    free(head);
    head = t;
    return item;
}

int main()
{
    STACK_init();
    STACK_push(4);
    STACK_push(3);
    STACK_push(2);
    STACK_push(1);
    while (!STACK_empty()) {
        printf("%d\n", STACK_pop());
    }
}
