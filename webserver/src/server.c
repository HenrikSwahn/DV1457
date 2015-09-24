#include "../include/server.h"
#include "../include/error.h"

Conf conf;

int handle_connection(int filedesc) {
	
	char buffer[512];
	int nbytes;
	
	nbytes = read(filedesc, buffer, 512);
	
	if(nbytes < 0) {
		perror("read error");
		exit(EXIT_FAILURE);
	}	
	else {
		parse_request(filedesc, buffer);
		return 1;	
	}
}

int create_server(uint16_t port) {

	conf = readConf();

	int sock;
	struct sockaddr_in server;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error creating sock");
		exit(EXIT_FAILURE);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(conf.port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("Error binding socket");
		exit(EXIT_FAILURE);
	}
	return sock;
}

void run_server() {
		
	int server_sock, index;
	fd_set active_fd_set, read_fd_set;
	struct sockaddr_in client;
	socklen_t sock_len;
	server_sock = create_server(PORT);
	
	if(listen(server_sock, 10000) < 0) {
		perror("Error listening");
		exit(EXIT_FAILURE);
	}

	FD_ZERO (&active_fd_set);
	FD_SET (server_sock, &active_fd_set);
	
	while(1) { 				
		read_fd_set = active_fd_set;
		if(select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			perror("select error");
			exit(EXIT_FAILURE);
		}
		// Service sockets with input 
		for(index = 0; index < FD_SETSIZE; index++) {
			if(FD_ISSET(index, &read_fd_set)) {
				if(index == server_sock) {
					int new_con;
					sock_len = sizeof(client);
					new_con = accept(server_sock, (struct sockaddr *) &client, &sock_len);
					if(new_con < 0) {
						perror("Error accepting new con");
						exit(EXIT_FAILURE);
					}
					fprintf(stderr, "Server: Connection from host: %hd, port: %hd. \n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
					FD_SET(new_con, &active_fd_set);
				}
				else {
					// Data on an already connected socket 
					if(handle_connection(index)) {
						close(index);
						FD_CLR(index, &active_fd_set);
					}
				}
			}
		}
	}
}

void parse_request(int socket, char * buffer) {
	
	//If GET request
	if(strstr(buffer, "GET") != NULL) {
		sendPage(socket);
	} else {
		write(socket, HTTP_NOT_IMPL, strlen(HTTP_NOT_IMPL));
	}
}

void sendPage(int filedesc) {

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
		strlen("\n") +  
		bodyLen;
	
	response = malloc(len);
 	
	strcat(response, HTTP_OK);
	strcat(response, HEADER_LANG);
	strcat(response, HEADER_CONT_TYPE);
	strcat(response, "\n");
	strcat(response, body);
	
	response[len] = '\0';
	write(filedesc, response, len); 

	free(body);
	free(response);
}

Conf readConf() {
	
	Conf c;
	FILE *file;
	char line[128];
	char buff[128];

	file = fopen("./test.config", "rt");
	if(file != NULL) {	
		while(fgets(line, 128, file) != NULL) {
			sscanf(line, "%[^\n]", buff);
			if(strstr(buff, "PORT") != NULL) {
				c.port = parsePort(buff);
			}else if(strstr(buff, "DIR") != NULL) {
				c.path=parseDir(buff);	
			}
		}
	}
	else {
		printf("ERROR");
	} 
	return c;
}

int parsePort(char arr[]) {
	
	int port = -1;
	sscanf(arr, "%*[^0123456789]%d", &port);
	
	if(port ==  -1) {
		printf("No port specified, setting to defulat, 12000");
		port = PORT;
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
