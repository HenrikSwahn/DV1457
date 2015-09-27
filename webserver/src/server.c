#include "../include/server.h"
#include "../include/error.h"

Conf conf;

int handle_connection(int filedesc) {
	
	char buffer[512];
	int counter;
	
	counter = read(filedesc, buffer, 512);
	
	if(counter < 0) {
		perror("read error");
		exit(EXIT_FAILURE);
	}	
	else {
		parse_request(filedesc, buffer);
		return 1;	
	}
}

int create_server(uint16_t port, int lPort) {

	conf = read_conf();

	int sock;
	struct sockaddr_in server;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		_error(SCREATE_ERROR);
	}

	server.sin_family = AF_INET;
	if(lPort != -1) {
		server.sin_port = htons(lPort);
	}
	else {
		server.sin_port = htons(conf.port);
	}
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		_error(SBIND_ERROR);
	}
	return sock;
}

void run_server(int lPort) {
	
	struct sockaddr_in client;		
	int server_sock;
	int index;
	fd_set file_set;
	fd_set ready_files;
	socklen_t sock_len;

	server_sock = create_server(PORT,lPort);
      
	if(listen(server_sock, MAXQ) < 0) {
		_error(SLISTEN_ERROR);
	}

	FD_ZERO (&file_set);
	FD_SET (server_sock, &file_set);
	
	while(1) { 		

		ready_files = file_set;

		if(select(FD_SETSIZE, &ready_files, NULL, NULL, NULL) < 0) {
			_error(SELECT_ERROR);
		}

		for(index = 0; index < FD_SETSIZE; index++) {

			if(FD_ISSET(index, &ready_files)) {

				if(index == server_sock) {
					int new_con;
					sock_len = sizeof(client);
					new_con = accept(server_sock, (struct sockaddr *) &client, &sock_len);

					if(new_con < 0) {
						_error(SACCEPT_ERROR);
					}

					fprintf(
						stderr, 
						"ip: %s, port: %d connected\n",
						inet_ntoa(client.sin_addr), 
						(int)ntohs(client.sin_port)
					);

					FD_SET(new_con, &file_set);
				}
				else { 
					if(handle_connection(index)) {
						puts("CLOSED");
						close(index);
						FD_CLR(index, &file_set);
					}
				}
			}
		}
	}
}

void parse_request(int socket, char * buffer) {
	
	char *token = strtok(buffer, " ");

	while(token) {
		////printf("token: %s\n", token);

		if(strcmp(token, "GET") == 0) {
			token = strtok(NULL, " ");
			if(strstr(token, "/") != NULL) {
				get_req(token, socket);
				break;
			}
		}
		else if(strcmp(token, "HEAD") == 0) {
			token = strtok(NULL, " ");
			if(strstr(token, "/") != NULL) {
				//head_req
				break;
			}
		}
		else {
			write(socket, HTTP_NOT_IMPL, strlen(HTTP_NOT_IMPL));
			break;
		}
		token = strtok(NULL, " ");
	}	
}

//Content length
//Content type
//modifed date
//server name

void get_req(char *path, int socket) {
	
	if(strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
		
		char *file_path = append_strings(BASE_DIR, "index.html");
		FILE *file = fopen(file_path, "r");

		if(file != NULL) {
			char * res = read_file(file);
			write(socket, res, strlen(res));
			fclose(file);
			free(res);
		}
		
		free(file_path);
	}else {

		char *file_path = append_strings(BASE_DIR, path+1);//+1 to get rid of first /
		FILE *file = fopen(file_path, "r");

		if(file != NULL) {
			char * res = read_file(file);
			write(socket, res, strlen(res));
			fclose(file);
			free(res);
		}
		else {
			write(socket, HTTP_NOT_FOUND, strlen(HTTP_NOT_FOUND));
		}
	}	
}

//Konstigt beteende
char *append_strings(char *s1, char *s2) {

	int s1_len = strlen(s1);
	int s2_len = strlen(s2);
	char *r = malloc(s1_len + s2_len + 1);

	memcpy(r, s1, s1_len);
	memcpy(r+s1_len, s2, s2_len+1);

	return r;
}

char * read_file(FILE *file) {

	char *response;
	size_t n = 0;
	int c;
	long file_size = -1;
	size_t head_len = -1;

	//Get size of file
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	head_len = strlen(HTTP_OK) +
		strlen(HEADER_CONT_TYPE) +
		strlen(HEADER_LANG) +
		strlen("\n");

	response = malloc(head_len + file_size + 1);

	strcpy(response, HTTP_OK);
	strcat(response, HEADER_LANG);
	strcat(response, HEADER_CONT_TYPE);
	strcat(response, "\n");

	n = head_len;

	while((c = fgetc(file)) != EOF) {
		response[n++] = (char)c;
	}

	response[n] = '\0';

	return response;
}

Conf read_conf() {
	
	Conf c;
	FILE *file;
	char line[128];
	char buff[128];

	file = fopen("./test.config", "rt");
	if(file != NULL) {	
		while(fgets(line, 128, file) != NULL) {
			sscanf(line, "%[^\n]", buff);
			if(strstr(buff, "PORT") != NULL) {
				c.port = parse_port(buff);
			}else if(strstr(buff, "DIR") != NULL) {
				c.path=parse_dir(buff);	
			}
		}
	}
	else {
		printf("ERROR");
	} 
	return c;
}

int parse_port(char arr[]) {
	
	int port = -1;
	sscanf(arr, "%*[^0123456789]%d", &port);
	
	if(port ==  -1) {
		printf("No port specified, setting to defulat, 12000");
		port = PORT;
	}
	return port;
}

char* parse_dir(char arr[]) {
	
	char * dir;

	dir = strstr(arr, "/");

	if(dir != NULL) {
		return(dir);
	}
	return 0;
}
