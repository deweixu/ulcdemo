
/**
** link node insert sort
*/

#include "link_node.h"

#define N 10

int main()
{
    struct node heada, headb;
    link t, u, x, a = &heada, b;
    int i;
    for (i = 0, t = a; i < N; i++) {
        t->next = malloc(sizeof *t);
        t = t->next;
        t->next = NULL;
        t->item = rand() % 100;
    }

    show_link(&heada);

    b = &headb;
    b->next = NULL;
    for (t = a->next; t != NULL; t = u) {
        u = t->next;
        for (x = b; x->next != NULL; x = x->next) {
            if (x->next->item > t->item)
                break;
        }
        t->next = x->next;
        x->next = t;
    }

    show_link(&headb);
}