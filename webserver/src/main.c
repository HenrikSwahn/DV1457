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

int main(int argc, char *argv[]) {

  int port = readArgs(argc, argv);
  run_server(port);
  return 0;
}




















