#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#define HTTP_OK "HTTP/1.0 200 OK\n"
#define HTTP_NOT_FOUND "HTTP/1.0 404 Not Found\n"
#define HTTP_NOT_IMPL "HTTP/1.0 501 Not Implemented\n"
#define HEADER_CONT_TYPE "Content-Type: text/html\n"
#define HEADER_LANG "Content-Language: en\n"
#define PORT 12000
#define MAXQ 10000
#define BASE_DIR "../www/"

typedef struct {
	uint16_t port;
	char* path;
	char* concurrency;
} Conf;

void run_server(int lPort);
int handle_connection(int);
int create_server(int);
void parse_request(int, char*);
void get_req(char *, int);
char * read_file(FILE*);
Conf read_conf();
int parse_port(char a[]);
char* parse_dir(char a[]);
char *append_strings(char *s1, char *s2);

#endif
