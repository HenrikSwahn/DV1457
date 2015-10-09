#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>

#include "error.h"
#include "http.h"
#include "conf.h"

void cleanup(int sig);
void make_daemon();
void run_server(int, int);
int handle_connection(int);
int create_server(int, int);
void parse_request(int, char*);

#endif
