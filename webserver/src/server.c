#include "../include/server.h"
#include "../include/error.h"
#include <limits.h>

Conf conf;
char * default_dir;

/*
 * Function that has the main server loop, it listen for 
 * new connection and adds the new sockets filedescriptor
 * to the set holding the filedescriptors. If data becomes
 * avaiable on an exsisting connection it calls a function
 * to handle the request
 * @PARAM {int lPort} Port number specified by program arguments
 */
void run_server(int lPort) {
	
	struct sockaddr_in client;		
	int server_sock;
	int index;
	fd_set file_set;
	fd_set ready_files;
	socklen_t sock_len;

	server_sock = create_server(lPort);
      
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

/*
 * Function to handle a connection is data becomes
 * avaiable. It reads the avaiable data, exits if 
 * read error, else calls to parse the data
 * @PARAM {int socket} server socket filedescriptor value
 * @RET returns 1 when handling of the request is done
 */
int handle_connection(int socket) {
	
	char buffer[512];
	int ret;
	
	ret = read(socket, buffer, 512);
	
	if(ret < 0) {
		perror("read error");
		exit(EXIT_FAILURE);
	}	
	else {
		parse_request(socket, buffer);
		return 1;	
	}
}

/*
 * Helper function that creates the server. 
 * It reads a config file for server port, 
 * base dir and concurrency method.
 * @PARAM {int lPort} Port number specified by program arguments 
 * @RETURN return an integer with the sockets filedescriptor value 
 */
int create_server(int lPort) {

	conf = read_conf();
	int sock;
	struct sockaddr_in server;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		_error(SCREATE_ERROR);
	}

	server.sin_family = AF_INET;
	if(lPort != -1) {
		server.sin_port = htons(lPort);
		printf("Server: Got port: %d from program argument, using it\n", lPort);
	}
	else if(conf.port > 1024) {
		server.sin_port = htons(conf.port);
		printf("Server: Got port: %d from config file, using it\n", conf.port);
	}else {
		server.sin_port = htons(PORT);
		printf("Server: No port specified, setting to system default: %d\n", PORT);
	}
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		_error(SBIND_ERROR);
	}
	return sock;
}

/*
 * A function that looks in the client request looking for 
 * GET or HEAD, if it cant find them it sends 501 to clients. 
 * @PARAM {int socket} server socket filedescriptor value
 * @PARAM {char * buffer} A buffer with clients request
 */
void parse_request(int socket, char * buffer) {
	
	char *token = strtok(buffer, " ");
	while(token) {

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

/*
 * Function to handle if the client made a GET request.
 * It first checks if the client want to acces /,
 * if so it sends that file if it can be found. If the client
 * want to access something els like "/index.html" the function 
 * tries to open that file, if fail returns not found to client
 * @PARAM {char *path} The path the client want to acces
 * @PARAM {int socket} server socket filedescriptor value
 */
void get_req(char *path, int socket) {

  if(strcmp(path, "/") == 0 ){
    char *file_path = append_strings(BASE_DIR, "/index.html");
    FILE *file = fopen(file_path, "r");
    
    if(file != NULL) {
      char * res = read_file(file);
      write(socket, res, strlen(res));
      fclose(file);
      free(res);
    }
    
    free(file_path);
  }else {
    char actualPath [PATH_MAX];
    char *str = append_strings(BASE_DIR,path);
    char * file_path = realpath(str,actualPath);
    
    if(file_path) {
      FILE *file = fopen(file_path, "r");

      char * res = read_file(file);
      write(socket, res, strlen(res));
      fclose(file);
      free(res);
      free(str);
    }
    else{
      write(socket, HTTP_NOT_FOUND, strlen(HTTP_NOT_FOUND));
    }
  }	
}

/*
 * Function that read the requested html page, 
 * builds a reponse and returns a pointer to
 * the response
 * @PARAM {FILE *file} The html page that the client has requested
 * @RETURN A pointer to the response
 */
char * read_file(FILE *file) {

	char *response;
	size_t n = 0;
	int c;
	long file_size = -1;
	size_t head_len = -1;
	char *cont_len;

	//Get size of file
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	cont_len = cont_length(file_size);

	head_len = strlen(HTTP_OK) +
		strlen(HEADER_CONT_TYPE) +
		strlen(HEADER_LANG) +
		strlen(cont_len) +
		strlen("\n");

	response = malloc(head_len + file_size + 1);

	strcpy(response, HTTP_OK);
	strcat(response, HEADER_LANG);
	strcat(response, HEADER_CONT_TYPE);
	strcat(response, cont_len);
	strcat(response, "\n");

	n = head_len;

	while((c = fgetc(file)) != EOF) {
		response[n++] = (char)c;
	}

	response[n] = '\0';

	free(cont_len);
	return response;
}

/*
 * Reads the config file, looking for port,
 * concurrency method and path to base dir
 * @RETURN A conf variable that will be assigned
 * to the servers global conf variable
 */
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

		if(strlen(c.path) == 0) {
			c.path = "N/A";
		}

		fclose(file);
	}
	else {
		printf("ERROR");
	} 
	return c;
}

/*
 * A helper function to the parsing of the conf file
 * I looks for a number in a string and returns what it found
 * @PARAM {char arr[]} The string that holds the port number
 * @RETRUN The found port number
 */
int parse_port(char arr[]) {
	
	int port = -1;
	sscanf(arr, "%*[^0123456789]%d", &port);
	return port;
}

/*
 * A helper function to the parsing of the conf file
 * I looks for a path in a string and returns what it found
 * @PARAM {char arr[]} The string that holds the path
 * @RETRUN The found path
 */
char* parse_dir(char arr[]) {
	
	char * dir;

	dir = strstr(arr, "/");

	if(dir != NULL) {
		return(dir);
	}
	return 0;
}

/*
 * A helper function that appends two string
 * @PARAM {char *s1} First string
 * @PARAM {char *s2} Second string
 * @RETURN A pointer to the complete string
 */
char *append_strings(char *s1, char *s2) {

	int s1_len = strlen(s1);
	int s2_len = strlen(s2);
	char *r = malloc(s1_len + s2_len + 1);

	memcpy(r, s1, s1_len);
	memcpy(r+s1_len, s2, s2_len+1);

	return r;
}

char *cont_length(long size) {

	char body_len[10];
	sprintf(body_len, "%ld\n", size);
	char *b_l = body_len;
	char * cont_len = append_strings(HEADER_CONT_LEN, b_l);
	return cont_len;
}
