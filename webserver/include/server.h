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
#define HTTP_NOT_IMPL "HTTP/1.0 501 Not implemented\n"
#define HEADER_CONT_TYPE "Content-Type: text/html\n"
#define HEADER_LANG "Content-Language: en\n"
#define PORT 12000
#define MAXQ 10000

typedef struct {
	uint16_t port;
	char* path;
	char* concurrency;
} Conf;

void run_server(int lPort);
void parse_request(int, char*);
Conf readConf();
int parsePort(char a[]);
char* parseDir(char a[]);
void sendPage(int);
int create_server(uint16_t, int lPort);
int handle_connection(int);

#endif
