#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#define MSGLEN 512
#define oops(m, x) {perror(m); exit(x);}
#define SOCKNAME "/tmp/logfilesock"

int main(int ac, char *av[])
{
    int sock;
    struct sockaddr_un addr;
    socklen_t addrlen;
    char msg[MSGLEN];
    int l;
    char sockname[] = SOCKNAME;
    time_t now;
    int msgnum = 0;
    char *timestr;

    /* build and address */
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sockname);
    addrlen = strlen(sockname) + sizeof(addr.sun_family);

    sock = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1)
        oops("socket", 2);
    /* bind the address */
    if (bind(sock, (struct sockaddr *)&addr, addrlen) == -1)
        oops("bind", 3);
    /* read and write */
    while (1) {
        l = read(sock, msg, MSGLEN);
        msg[l] = '\0';
        time(&now);
        timestr = ctime(&now);
        timestr[strlen(timestr) - 1] = '\0';

        printf("[%5d] %s %s\n", msgnum++, timestr, msg);
        fflush(stdout);
    }
}