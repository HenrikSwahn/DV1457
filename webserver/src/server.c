#include "../include/server.h"
#include "../include/error.h"

Server server;
Client client;

//Creates the socket and binds in to an address
int create_server() {
	
	readConf();	
	//conn.buffsize = 1024;
	//conn.buffer = malloc(conn.buffsize);	
		
	//Create the socket
	if ((server.sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		_error(SCREATE_ERROR);
	}
	
	printf("Socket was created successfully\n");

	server.address.sin_family = AF_INET;
	server.address.sin_addr.s_addr = INADDR_ANY;

	//Bind the socket to an address
	if (bind(server.sock, (struct sockaddr *) &server.address, sizeof(server.address)) <  0) {
		_error(SBIND_ERROR);
	}

	printf("Socket was bound successfully\n");
	return 0;
}

//Starts to listen on the socket and accepting if anyone tries to connect
int run_server() {
	
	client.buffsize = 1024;
	client.buffer = malloc(client.buffsize);	
	while(1) {
		//Start to listen, max queue 10
		if (listen(server.sock, 10) < 0) {
			_error(SLISTEN_ERROR);
		}
		
		//Accepts if a connection is made
		if ((client.sock = accept(server.sock, (struct sockaddr *) &server.address, &server.addrlen)) < 0) {
			_error(SACCEPT_ERROR);
		}	
		
		//Checks if the client is connected
		if (client.sock > 0) {
			printf("Client is connected\n");
		}
		
		//Recieve data from the client
		recv(client.sock, client.buffer, client.buffsize, 0);
		printf("%s\n", client.buffer);
		
		//Parse incomming request
		parseRequest(client.buffer);
		
		//Close the client socket
		close(client.sock);
	}
}

int close_server() {
	close(server.sock);
	return 0;
}

void parseRequest(char * buffer) {
	
	//If GET request
	if(strstr(buffer, "GET") != NULL) {
		sendPage();
	} else {
		write(client.sock, HTTP_NOT_IMPL, strlen(HTTP_NOT_IMPL));
	}
}

void sendPage() {

	FILE *file = fopen("../www/index.html", "rt");
	char * body;
	size_t bodyLen = 0;
	int c;

	body = malloc(1024);
	while((c = fgetc(file)) != EOF) {
		body[bodyLen++] = (char) c;
	}
	
	body[bodyLen] = '\0';
	
	char * response;
	size_t len = strlen(HTTP_OK) +
		strlen(HEADER_CONT_TYPE) +
		strlen(HEADER_LANG) +
		strlen(NEWLINE) +  
		bodyLen;
	
	response = malloc(len);
 	
	strcat(response, HTTP_OK);
	strcat(response, HEADER_LANG);
	strcat(response, HEADER_CONT_TYPE);
	strcat(response, NEWLINE);
	strcat(response, body);
	
	response[len] = '\0';
	write(client.sock, response, len); 

	free(body);
	free(response);
}

void readConf() {

	FILE *file;
	char line[128];
	char buff[128];

	file = fopen("./test.config", "rt");
	if(file != NULL) {	
		while(fgets(line, 128, file) != NULL) {
			sscanf(line, "%[^\n]", buff);
			if(strstr(buff, "PORT") != NULL) {
				server.address.sin_port = htons(parsePort(buff));
			}else if(strstr(buff, "DIR") != NULL) {
				parseDir(buff);	
			}
		}
	}
	else {
		printf("ERROR");
	} 
}

int parsePort(char arr[]) {
	
	int port = -1;
	sscanf(arr, "%*[^0123456789]%d", &port);
	
	if(port ==  -1) {
		printf("No port specified, setting to defulat, 12000");
		port = 12000;
	}
	return port;
}

char* parseDir(char arr[]) {
	
	char * dir;

	dir = strstr(arr, "/");

	if(dir != NULL) {
		return(dir);
	}
	return 0;
}
