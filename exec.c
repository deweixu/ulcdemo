#include <stdio.h>
#include <unistd.h>

int main()
{
    char *arglist[2];

    arglist[0] = "ls";
    arglist[1] = NULL;
    printf("*** About to exec ls - l\n");
    execvp("ls", arglist);
    printf("*** ls is done. bye\n");
}