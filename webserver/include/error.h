#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdio.h>

#define SACCEPT_ERROR -1
#define SLISTEN_ERROR -2
#define SCREATE_ERROR -3
#define SBIND_ERROR -4
#define SELECT_ERROR -5

void _error(int error);
void _socket_accept_error();
void _socket_listen_error();
void _socket_bind_error();
void _socket_create_error();
void _select_error();

#endif