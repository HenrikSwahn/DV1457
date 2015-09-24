#include "../include/error.h"

void _error(int error) {

	switch(error) {
		case SACCEPT_ERROR:
			_socket_accept_error();
			break;
		case SLISTEN_ERROR:
			_socket_listen_error();
			break;
		case SCREATE_ERROR:
			_socket_create_error();
			break;
		case SBIND_ERROR:
			_socket_bind_error();
			break;
	}
}

void _socket_accept_error() {
	perror("creating");
	exit(EXIT_FAILURE);
}

void _socket_bind_error() {
	perror("binding");
	exit(EXIT_FAILURE);
} 

void _socket_listen_error() {
	perror("listening");
	exit(EXIT_FAILURE);
}

void _socket_create_error() {
	perror("create");
	exit(EXIT_FAILURE);
}

void _select_error() {
	perror("select");
	exit(EXIT_FAILURE);
}