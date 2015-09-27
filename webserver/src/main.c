#include <signal.h>
#include "../include/server.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

void printHelpText() {
	printf("\nUsage of server: \n-h  prints help text\n-p  set port server will listen to\n\n");
}

int readArgs(int nr, char *args[]) {
	int arg;
	int port = -1;
  
	while((arg = getopt(nr,args,"hp:")) !=-1) {
		switch (arg) {
			case 'h':
				printHelpText();
				exit(0);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			default:
				printHelpText();
				exit(3);
      	}
	}
	return port;
}

/*void cleanup(int sig) {

	printf("\n\nStarting cleanup\n");
	printf("Closing sockets\n");

	shutdown(conn.client_socket, 2);
	printf("Closed client_socket\n");

	shutdown(conn.server_socket, 2);
	printf("Closed server_socket\n");

	free(conn.buffer);
	printf("Released memory hold by buffer\n");
	exit(0);
}*/

int main(int argc, char *argv[]) {

  int port = readArgs(argc, argv);
  run_server(port);
  return 0;
}




















