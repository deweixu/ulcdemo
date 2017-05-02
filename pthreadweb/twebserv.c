#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

void *handle_call(void *);
int make_server_socket(int);
void setup(pthread_attr_t *);
void skip_rest_of_header(FILE *);
void process_rq(char *, int);
void sanitize(char *);
int bulit_in(char *, int);
int http_reply(int, FILE **, int, char *, char *, char *);
void not_implemented(int);
void do_404(char *, int); 
int isadir(char *);
int not_exist(char *);
void do_ls(char *, int);
char *file_type(char *);
void do_cat(char *, int);

time_t server_started;
int server_byte_sent;
int server_requests;

int main(int ac, char *av[])
{
    int sock, fd;
    int *fdptr;
    pthread_t worker;
    pthread_attr_t attr;

    if (ac == 1) {
        fprintf(stderr, "useages: tws portnum\n");
        exit(1);
    }
    sock = make_server_socket(atoi(av[1]));
    if (sock == -1) {
        perror("making socket");
        exit(2);
    }
    setup(&attr);

    while (1) {
        fd = accept(sock, NULL, NULL);
        server_requests ++;
        fdptr = malloc(sizeof(int));
        *fdptr = fd;
        pthread_create(&worker, &attr, handle_call, fdptr);
    }
}

/*
* initialize the status variables and
* set the thread attribute to detached
*/
void setup(pthread_attr_t *attrp)
{
    pthread_attr_init(attrp);
    pthread_attr_setdetachstate(attrp, PTHREAD_CREATE_DETACHED);

    time(&server_started);
    server_requests = 0;
    server_byte_sent = 0;
}

void *handle_call(void *fdptr)
{
    FILE *fpin;
    char request[BUFSIZ];
    int fd;

    fd = *(int *)fdptr;
    free(fdptr);

    fpin = fdopen(fd, "r");
    fgets(request, BUFSIZ, fpin);
    printf("got a call on %d: request = %s", fd, request);
    skip_rest_of_header(fpin);

    process_rq(request, fd);
    fclose(fpin);
    return 0;
}

/*
* skip_rest_of_header(FILE *)
* skip over request info until a CRNL is seen
*/
void skip_rest_of_header(FILE *fp)
{
    char buf[BUFSIZ] = {};
    while (fgets(buf, BUFSIZ, fp) != NULL && strcmp(buf, "\r\n") != 0);

}

/*
* process_rq(char *rq, int fd)
* do what the request asks for and write reply th fd
* handles request in a new process
* rq is HTTP command: GET /foo/bar.html HTTP/1.0
*/
void process_rq(char * rq, int fd)
{
    printf("rq: %s\n", rq);
    char cmd[BUFSIZ], arg[BUFSIZ];

    if (sscanf(rq, "%s %s", cmd, arg) != 2)
        return;

    printf("cmd: %s\n", cmd);
    printf("arg: %s\n", arg);
    sanitize(arg);
    printf("sanitize arg: %s\n", arg);
    printf("sanitized version is %s\n", arg);

    if (strcmp(cmd, "GET") != 0) {
        not_implemented(fd);
    } else if (bulit_in(arg, fd)) {
        
    } else if (not_exist(arg)) {
        do_404(arg, fd);
    } else if (isadir(arg)) {
        do_ls(arg, fd);
    } else {
        do_cat(arg, fd);
    }
}

/*
* make sure all paths are below the current directory
*/
void sanitize(char *str)
{
    char *src, *dest;
    src = dest = str;

    while (*src) {
        if (strncmp(src, "/../", 4) == 0) {
            src += 3;
        } else if (strncmp(src, "//", 2) == 0) {
            src ++;
        } else {
            *dest++ = *src++;
        }
    }
        *dest = '\0';
        if (*str == '/') {
            char *temp = malloc(sizeof(char) * strlen(str));
            strcpy(temp, str + 1);
            strcpy(str, temp);
            free(temp);
        }
        if (str[0] == '\0' || strcmp(str, "./") == 0
            || strcmp(str, "./..") == 0) {
                strcpy(str, ".");
        }
}

/*
* handle built-in URLs here. Only one so far is "status"
*/
int bulit_in(char *arg, int fd)
{
    FILE *fp;

    if (strcmp(arg, "status") != 0)
        return 0;
    http_reply(fd, &fp, 200, "OK", "text/plain", NULL);

    fprintf(fp, "Server started: %s", ctime(&server_started));
    fprintf(fp, "Total requests: %d\n", server_requests);
    fprintf(fp, "Bytes sent out: %d\n", server_byte_sent);
    fclose(fp);
    return 1;
}

int http_reply(int fd, FILE **fpp, int code, char *msg, char *type, char *content)
{
    FILE *fp = fdopen(fd, "w");
    int bytes = 0;

    if (fp != NULL) {
        bytes = fprintf(fp, "HTTP/1.0 %d %s\r\n", code, msg);
        bytes += fprintf(fp, "Content-type: %s\r\n\r\n", type);
        if (content) 
            bytes += fprintf(fp, "%s\r\n", content);
    }
    fflush(fp);
    if (fpp) {
        *fpp = fp;
    } else {
        fclose(fp);
    }
    return bytes;      
}

void not_implemented(int fd)
{
    http_reply(fd, NULL, 501, "Not Implemented", "text/plain",
        "That command is not implemented");
}

void do_404(char *item, int fd) 
{
    http_reply(fd, NULL, 404, "Not Found", "text/plain",
        "The item you seek is not here");
}

int isadir(char *f)
{
    struct stat info;
    return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode));
}

int not_exist(char *f)
{
    struct stat info;
    return (stat(f, &info) == -1);
}

void do_ls(char *dir, int fd)
{
    DIR *dirptr;
    struct dirent *direntp;
    FILE *fp;
    int bytes = 0;

    bytes = http_reply(fd, &fp, 200, "OK", "text/plain", NULL);
    bytes += fprintf(fp, "Listing of Directory %s\n", dir);

    if ((dirptr = opendir(dir)) != NULL) {
        while ( (direntp = readdir(dirptr)) != NULL ) {
            bytes += fprintf(fp, "%s\n", direntp->d_name);
        }
        closedir(dirptr);
    }
    fclose(fp);
    server_byte_sent += bytes;
}

char *file_type(char *f)
{
    char *cp;
    if ((cp = strrchr(f, '.')) != NULL)
        return cp + 1;
    return " -";
}

void do_cat(char *f, int fd)
{
    char *extension = file_type(f);
    char *content = "text/plain";
    FILE *fpsock, *fpfile;
    int c;
    int bytes = 0;

    if ( strcmp(extension, "html") == 0 ) {
        content = "text/html";
    } else if (strcmp(extension, "gif") == 0 ) {
        content = "image/gif";
    } else if ( strcmp(extension, "jpg") == 0 ) {
        content = "image/jpeg";
    } else if (strcmp(extension, "jpeg") == 0) {
        content = "image/jpeg";
    }

    fpsock = fdopen(fd, "w");
    fpfile = fopen(f, "r");
    if (fpsock != NULL && fpfile != NULL) {
        bytes = http_reply(fd, &fpsock, 200, "OK", content, NULL);
        while( (c = getc(fpfile)) != EOF ){
            putc(c, fpsock);
            bytes ++;
        }
        fclose(fpfile);
        fclose(fpsock);
    }
    server_byte_sent += bytes;
}