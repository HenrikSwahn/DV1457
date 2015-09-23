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

#define HTTP_OK "HTTP/1.0 200 OK\n"
#define HTTP_NOT_IMPL "HTTP/1.0 501 Not implemented\n"
#define HEADER_CONT_TYPE "Content-Type: text/html\n"
#define HEADER_LANG "Content-Language: en\n"
#define NEWLINE "\n"

typedef struct {
	int sock;
	//int client_socket;
	//int buffsize;
	//char *buffer;
	struct sockaddr_in address;	
	socklen_t addrlen;
} Server;

typedef struct {
	int sock;
	int buffsize;
	char *buffer;	
} Client;

int create_server(); 
int run_server();
void parseRequest(char * buffer);
int close_server();
void readConf();
int parsePort(char arr[]);
char* parseDir(char arr[]);
void sendPage();

#endif
