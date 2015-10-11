#ifndef CONF_H
#define CONF_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#define PORT 12000
#define CONCURRENCY "MULTIPLEXING"
#define BASE_DIR "../www"

typedef struct {
	uint16_t port;
	int daemon;
	char* path;
	char* concurrency;
} Conf;

Conf * read_conf(int);
int parse_port(char *);
char * parse_dir(char *);
char * parse_concurrency(char *);

#endif