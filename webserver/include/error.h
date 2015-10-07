#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#define SACCEPT_ERROR -1
#define SLISTEN_ERROR -2
#define SCREATE_ERROR -3
#define SBIND_ERROR -4
#define SELECT_ERROR -5
#define FORK_ERROR -6
#define SESSION_ERROR -7
#define CHANGE_DIR_ERROR -8
#define READ_ERROR -9
#define LOGFILE_FORMAT_ERROR -10

void _error(int error);
void _socket_accept_error();
void _socket_listen_error();
void _socket_bind_error();
void _socket_create_error();
void _select_error();
void _fork_error();
void _session_error();
void _change_dir_error();
void _read_error();
void _logfile_format_error();

#endif