#include "link_node.h"

void show_link(link head)
{
    link x = head;
    while (x != NULL) {
        printf("%d\t", x->item);
        x = x->next;
    }
    printf("Done.\n");
}