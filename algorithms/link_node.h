#include <stdio.h>
#include <stdlib.h>

struct node {
    int item;
    struct node *next;
};

typedef struct node* link;

void show_link(link);