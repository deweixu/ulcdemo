#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
 #include <unistd.h>

#define SERVER_PORTNUM 2020
#define MSGLEN 128
#define TICKET_AVALL 0
#define MAXUSERS 3
#define oops(x) {perror(x); exit(1);}

int make_dgram_server_socket(int);
void free_all_tickets();
void narrate(char *, char *, struct sockaddr_in *);

/*
** Important variable
*/
int ticket_array[MAXUSERS];
int sd = -1;
int num_tickets_out = 0;
char *do_hello();
char *do_goodbye();

int setup()
{
    sd = make_dgram_server_socket(SERVER_PORTNUM);
    if ( sd == -1) 
        oops("make socket");
    free_all_tickets();
    return sd;
}

void free_all_tickets() 
{
    int i;
    for (i = 0; i < MAXUSERS; i++)
        ticket_array[i] = TICKET_AVALL;
}

void shut_down() 
{
    close(sd);
}

void handle_request(char *req, struct sockaddr_in *client, socklen_t addlen)
{
    char *response;
    int ret;

    /* act and compose a response */
    if ( strncmp(req, "HELO", 4) == 0 ) {
        response = do_hello(req);
    } else if ( strncmp(req, "GBYE", 4) == 0 ) {
        response = do_goodbye(req);
    } else {
        response = "FAIL invalid request";
    }
    /* send the response to the client */
    narrate("SAID:", response, client);
    ret = sendto(sd, response, strlen(response), 0, (struct sockaddr*)client, addlen);
    if (ret == -1)
        perror("SERVER sendto failed");
}

/*
** do_hello
** Give out a ticket if any are available
** IN msg_p     message received from client
** Results: ptr to response
** note: return is in static buffer verwritten by each call
*/
char *do_hello(char *msg_p)
{
    int x;
    static char replybuf[MSGLEN];

    if ( num_tickets_out >= MAXUSERS )
        return "FAIL no tickets available";
    /* else find a free ticket and give it to clietn */
    for (x = 0; x < MAXUSERS && ticket_array[x] != TICKET_AVALL; x++);

    /* A anity check - should never happen */
    if (x == MAXUSERS) {
        narrate("database corrupt", "", NULL);
        return "FAIL database corrupt";
    }

    ticket_array[x] = atoi(msg_p + 5);
    sprintf(replybuf, "TICK %d. %d", ticket_array[x], x);
    num_tickets_out++;
    return replybuf;
}

/*
** do_goodbye
** Take back ticket client is returning
** IN msg_p     message received from client
** Results: ptr to response
** Note: return is in static buffer over written by each call
*/
char *do_goodbye(char *msg_p)
{
    int pid, slot;

    if ((sscanf((msg_p + 5), "%d. %d", &pid, &slot) != 2) ||
        (ticket_array[slot] != pid)) {
            narrate("Bogus ticket", msg_p + 5, NULL);
            return "FAIL invalid ticket";
        }
    ticket_array[slot] = TICKET_AVALL;
    num_tickets_out--;
    return "THNX See ya";
}

void narrate(char *msg1, char *msg2, struct sockaddr_in *clientp)
{
    fprintf(stderr, "\t\tSERVER: %s %s", msg1, msg2);
    if (clientp) {
        fprintf(stderr, "(%s:%d)", inet_ntoa(clientp->sin_addr),
            ntohs(clientp->sin_port));
    }
    putc('\n', stderr);
}