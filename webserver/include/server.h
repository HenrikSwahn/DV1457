#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <syslog.h>


#define HTTP_OK "HTTP/1.0 200 OK\n"
#define HTTP_NOT_FOUND "HTTP/1.0 404 Not Found\n"
#define HTTP_NOT_IMPL "HTTP/1.0 501 Not Implemented\n"
#define HEADER_CONT_TYPE "Content-Type: text/html\n"
#define HEADER_LANG "Content-Language: en\n"
#define HEADER_CONT_LEN "Content-Length: "
#define HEADER_LAST_MOD "Last-Modified: "
#define HEADER_SERV_NAME "Server: Henrik/Andreas 1.0\n"
#define PORT 12000
#define CONCURRENCY "MULTIPLEXING"
#define BASE_DIR "../www"
#define MAXQ 10000

typedef struct {
	uint16_t port;
	int daemon;
	char* path;
	char* concurrency;
} Conf;

void cleanup(int sig);
void make_daemon();
void run_server(int, int);
int handle_connection(int);
int create_server(int, int);
void parse_request(int, char*);
void get_req(char *, int);
void head_req(char *, int);
char * read_file(FILE*, char *, char *, int);
Conf * read_conf(int);
int parse_port(char *);
char * parse_dir(char *);
char * parse_concurrency(char *);
char * append_strings(char *s1, char *s2);
char * build_headers(long, char *, int);
char * cont_length(long);
char * mod_date(char *);

#endif
