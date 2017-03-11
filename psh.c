#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXARGS 20
#define ARGLEN 100

char *makestring(char*);
void execute(char*[]);

int main()
{
    char *arglist[MAXARGS + 1];
    int numargs;
    char argbuf[ARGLEN];
    numargs = 0;

    while(numargs < MAXARGS){
        printf("Arg[%d]", numargs);
        if(fgets(argbuf, ARGLEN, stdin) && *argbuf != '\n'){
            arglist[numargs++] = makestring(argbuf);
        } else{
            if(numargs > 0){
                arglist[numargs] = NULL;
                execute(arglist);
                numargs = 0;
            }
        }
    }
}

void execute(char *arglist[])
{
    int pid, existstatus;
    pid = fork();
    switch( pid ){
        case -1:
            perror("fork failed!");
            exit(1);
        case 0:
            execvp(arglist[0], arglist);
            perror("execvp failed");
            exit(1);
        default:
            wait(&existstatus);
            printf("child exited with status %d, %d\n", existstatus >> 8,
                existstatus & 0377);
    }

}

char *makestring(char *buf)
{
    char *cp = NULL;
    buf[strlen(buf) - 1] = '\0';
    cp = (char*) malloc(strlen(buf) + 1);
    if(cp == NULL){
        fprintf(stderr, "no memory\n");
        exit(1);
    }
    strcpy(cp, buf);
    return cp;
}