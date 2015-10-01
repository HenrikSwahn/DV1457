#include "../include/server.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

typedef struct {
	int daemon;
	int port;
}Arguments;

void printHelpText() {
	printf("\nUsage of server: \n-h  prints help text\n-p  set port server will listen to\n\n");
}

Arguments readArgs(int nr, char *args[]) {
	int arg;
	Arguments a;
	a.port = -1;
	a.daemon = -1;
  
	while((arg = getopt(nr,args,"hp:d")) !=-1) {
		switch (arg) {
			case 'h':
				printHelpText();
				exit(0);
				break;
			case 'p':
				a.port = atoi(optarg);
				break;
			case 'd':
				a.daemon = 1;
				break;
			default:
				printHelpText();
				exit(3);
      	}
	}
	return a;
}

int main(int argc, char *argv[]) {

	Arguments a = readArgs(argc, argv);
	run_server(a.port, a.daemon);
	
	return 0;

}




















