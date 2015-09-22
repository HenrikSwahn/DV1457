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
	printf("Error, accepting\n");
	exit(EXIT_FAILURE);
}

void _socket_bind_error() {
	printf("Error binding socket\n");
	exit(EXIT_FAILURE);
} 

void _socket_listen_error() {
	printf("Error, listening\n");
	exit(EXIT_FAILURE);
}

void _socket_create_error() {
	printf("Error creating socket\n");
	exit(EXIT_FAILURE);
}