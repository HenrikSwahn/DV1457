#include <signal.h>
#include "../include/server.h"

void cleanup(int sig) {

	printf("Starting cleanup\n");
	printf("Closing sockets\n");

	shutdown(conn.client_socket, 2);
	printf("Closed client_socket\n");

	shutdown(conn.server_socket, 2);
	printf("Closed server_socket\n");

	free(conn.buffer);
	printf("Released memory hold by buffer\n");
	exit(0);
}

int main() {

	signal(SIGINT, cleanup);

	create_server();
	run_server();
	close_server();
	return 0;
}




















