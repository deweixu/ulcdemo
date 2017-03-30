#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

/*
** Important variable used throughout
*/
static int pid = -1;
static int sd = -1;
static struct sockaddr serv_addr;
static socklen_t serv_alen;
static char ticket_buf[128];
static int have_ticket = 0;

#define MSGLEN 128
#define SERVER_PORTNUM 2020
#define HOSTLEN 512
#define oops(p) {perror(p); exit(1);}

char *do_transaction();
void narrate(char *, char *);
void syserr(char *);

int make_dgram_client_socket();
int make_internet_address(char *, int , struct sockaddr_in *);

/*
** setup: get pid, socket, and address of license server
** IN no args
** RET nothing, dies on error
** notes: assumes server is on same host as client
*/
void setup()
{
    char hostname[BUFSIZ];

    pid = getpid();
    sd = make_dgram_client_socket();
    if (sd == -1)
        oops("Can`t create socket");
    gethostname(hostname, HOSTLEN);
    make_internet_address(hostname, SERVER_PORTNUM, (struct sockaddr_in *)&serv_addr);
    serv_alen = sizeof(serv_addr);
}

void shut_down()
{
    close(sd);
}

/*
** get_ticket
** get a ticket from the license server
** Results: 0 for success, -1 for fail
*/
int get_ticket() 
{
    char *response;
    char buf[MSGLEN];

    if (have_ticket)
        return 0;
    sprintf(buf, "HELO %d", pid);

    if ((response = do_transaction(buf)) == NULL)
        return -1;
    
    /*
    ** parse the response and see if we got a ticket
    ** on success, the message is: TICK ticket-string
    ** on fail, the message is: FAIL fail-msg
    */
    if (strncmp(response, "TICK", 4) == 0) {
        strcpy(ticket_buf, response + 5);
        have_ticket = 1;
        narrate("got ticket", ticket_buf);
        return 0;
    }
    if (strncmp(response, "FAIL", 4) == 0) {
        narrate("Could not get ticket", response);
    } else {
        narrate("Unkonwn message:", response);
    }
    return -1;
}

/*
** release_ticket
** Give ticket back to the server
** Results: 0 , -1
*/
int release_ticket()
{
    char buf[MSGLEN];
    char *response;

    if (! have_ticket)
        return 0;

    sprintf(buf, "GBYE %s", ticket_buf);
    if ((response = do_transaction(buf)) == NULL)
        return -1;

    if (strncmp(response, "THNX", 4) == 0) {
        narrate("release ticket OK", "");
        return 0;
    }

    if (strncmp(response, "FAIL", 4) == 0) {
        narrate("release failed", response+5);
    } else{
        narrate("Unkonwn message:", response);
    }
    return -1;
}

/*
** do_transaction
** Send a request to the server and get a response back
** IN msg_p         message to send
** Results: pointer to message string, or NULL for error
**                  Note: pointer returned is to static storage
**                  overwritten by each successive call
** noteL for extra security, compare retaddr to serv_addr
*/
char *do_transaction(char *msg)
{
    static char buf[MSGLEN];
    struct sockaddr retaddr;
    socklen_t addrlen = sizeof(retaddr);
    int ret;

    ret = sendto(sd, msg, strlen(msg), 0, &serv_addr, serv_alen);
    if (ret == -1) {
        syserr("sendto");
        return NULL;
    }
    /* get response back */
    ret = recvfrom(sd, buf, MSGLEN, 0, &retaddr, &addrlen);
    if (ret == -1) {
        syserr("recvfrom");
        return NULL;
    }
    return buf;
}

void narrate(char *msg1, char *msg2)
{
    fprintf(stderr, "CLIENT [%d] : %s %s", pid, msg1, msg2);
}

void syserr(char *msg)
{
    char buf[MSGLEN];
    sprintf(buf, "CLENT [%d]: %s", pid, msg);
    perror(buf);
}