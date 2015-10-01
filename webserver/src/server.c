#include "../include/server.h"
#include "../include/error.h"

Conf * conf;
char * default_dir;

void cleanup(int sig) {

	printf("\nCleaning up...\n");
	printf("Releasing:\n");
	
	printf("\t%s, in %p\n", conf->path, conf->path);
	free(conf->path);
	
	printf("\t%s, in %p\n", conf->concurrency, conf->concurrency);
	free(conf->concurrency);

	printf("\tConf struct, in %p\n", conf);
	free(conf);

	printf("Shuting down, bye\n");
	exit(EXIT_SUCCESS);
}

void make_daemon() {

	char actual_path[PATH_MAX + 1];
	char * full_path = realpath(conf->path, actual_path);
	free(conf->path);

	conf->path = malloc(strlen(full_path));
	strncpy(conf->path, full_path, strlen(full_path));
	
	pid_t p_id = 0;
	p_id = fork();

	if(p_id < 0) {
		exit(EXIT_FAILURE);
	}

	if(p_id > 0) {
		exit(EXIT_SUCCESS);
	}

	umask(0);

	openlog("Server" , LOG_NOWAIT | LOG_PID, LOG_USER); 
	syslog(LOG_NOTICE, "Successfully started daemon\n");

	pid_t sid;
	sid = setsid();

	if(sid < 0) {
		syslog(LOG_ERR, "Could not create process group\n");
		exit(EXIT_FAILURE);
	}

	if((chdir("/")) < 0) {
		syslog(LOG_ERR, "Could not change dir\n");
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	open("/dev/null", O_RDWR);
	dup(0);
	dup(0);
}

/*
 * Function that has the main server loop, it listen for 
 * new connection and adds the new sockets filedescriptor
 * to the set holding the filedescriptors. If data becomes
 * avaiable on an exsisting connection it calls a function
 * to handle the request
 * @PARAM {int lPort} Port number specified by program arguments
 */
void run_server(int lPort, int daemon) {
	
	signal(SIGINT, cleanup);
	struct sockaddr_in client;		
	int server_sock;
	int index;
	fd_set file_set;
	fd_set ready_files;
	socklen_t sock_len;

	server_sock = create_server(lPort, daemon);

	if(conf->daemon == 1) {
		make_daemon();
	}
      
	if(listen(server_sock, MAXQ) < 0) {
		_error(SLISTEN_ERROR);
	}

	if(conf->daemon == -1) {
		printf("%s%s\n", 
		"__________________________________________________________________________\n",
		"|______________Henrik_and_Andreas_webserver_is_firing up!!_______________|\n"); 
		printf("Server started listening on port: %d\nMain dir is: %s\nConcurrency method is set to: %s\n\n", conf->port, conf->path, conf->concurrency);
		printf("Wating for connection....\n");
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

					if(conf->daemon == -1) {
						printf("\t%s:%d connected.\n", inet_ntoa(client.sin_addr), (int)ntohs(client.sin_port));
					}

					FD_SET(new_con, &file_set);
				}
				else { 
					if(handle_connection(index)) {
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
	
	char * buffer = malloc(1024);
	int ret;
	
	ret = read(socket, buffer, 1024);
	
	if(ret < 0) {
		free(buffer);
		perror("read error");
		exit(EXIT_FAILURE);
	}	
	else {
		parse_request(socket, buffer);
		free(buffer);
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
int create_server(int lPort, int daemon) {

	conf = read_conf(daemon);
	int sock;
	struct sockaddr_in server;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		_error(SCREATE_ERROR);
	}

	server.sin_family = AF_INET;
	if(lPort != -1) {
		conf->port = lPort; 
		server.sin_port = htons(conf->port);

		if(conf->daemon == -1) {
			printf("Server: Got port: %d from program argument, overriding system default/config file port\n", conf->port);
		}
	}
	else {
		server.sin_port = htons(conf->port);
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
			if(token != NULL) {
				if(strstr(token, "/") != NULL) {

					if(conf->daemon == -1) {
						printf("\t\tRequesting: %s\n", token);
					}

					get_req(token, socket);
					break;
				}
			}
		}
		else if(strcmp(token, "HEAD") == 0) {
			token = strtok(NULL, " ");
			if(token != NULL) {
				if(strstr(token, "/") != NULL) {


					if(conf->daemon == -1) {
						printf("\t\tRequesting: %s\n", token);
					}

					head_req(token, socket);
					break;
				}
			}
		}
		else {
			write(socket, HTTP_NOT_IMPL, strlen(HTTP_NOT_IMPL));
			break;
		}
		token = strtok(NULL, " ");
	}	
}

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
 		char actualPath [PATH_MAX];
 		char * str = append_strings(conf->path , "/index.html");
 		char * real_file_path = realpath(str,actualPath);

 		if(real_file_path) {
 			FILE *file = fopen(real_file_path, "r");

 			char * res = read_file(file, real_file_path, "GET", 200);
 			write(socket, res, strlen(res));
 			fclose(file);
 			free(res);
 		}
 		else{
 			char *file_path = append_strings(conf->path, "/404.html");
 			FILE *file = fopen(file_path, "r");

 			if(file != NULL) {
 				char * res = read_file(file, file_path, "GET", 404);
 				write(socket, res, strlen(res));
 				fclose(file);
 				free(res);
 			}
 			free(file_path);
 			free(real_file_path);
 		}
 		free(str);
 	}else {
 		char actualPath [PATH_MAX];
 		char * str = append_strings(conf->path ,path);
 		char * real_file_path = realpath(str,actualPath);

 		if(real_file_path) {
 			FILE *file = fopen(real_file_path, "r");

 			char * res = read_file(file, real_file_path, "GET", 200);
 			write(socket, res, strlen(res));
 			fclose(file);
 			free(res);
 		}
 		else{
 			char *file_path = append_strings(conf->path, "/404.html");
 			FILE *file = fopen(file_path, "r");

 			if(file != NULL) {
 				char * res = read_file(file, file_path, "GET", 404);
 				write(socket, res, strlen(res));
 				fclose(file);
 				free(res);
 			}
 			free(file_path);
 			free(real_file_path);
 		}
 		free(str);	
 	}	
 }

/*
 * Function to handle if the client made a HEAD request.
 * @PARAM {char *path} The path the client want to acces
 * @PARAM {int socket} server socket filedescriptor value
 */
 void head_req(char *path, int socket) {

 	if(strcmp(path, "/") == 0 ){
 		char actualPath [PATH_MAX];
 		char *str = append_strings(conf->path, "/index.html");
 		char * real_file_path = realpath(str,actualPath);

 		if(real_file_path) {
 			FILE *file = fopen(real_file_path, "r");

 			char * res = read_file(file, real_file_path, "HEAD", 200);
 			write(socket, res, strlen(res));
 			fclose(file);
 			free(res);
 		}
 		else{
 			char *file_path = append_strings(conf->path, "/404.html");
 			FILE *file = fopen(file_path, "r");

 			if(file != NULL) {
 				char * res = read_file(file, file_path, "HEAD", 404);
 				write(socket, res, strlen(res));
 				fclose(file);
 				free(res);
 			}
 			free(file_path);
 			free(real_file_path);
 		}
 		free(str);
 	}else {
 		char actualPath [PATH_MAX];
 		char *str = append_strings(conf->path, path);
 		char * real_file_path = realpath(str,actualPath);

 		if(real_file_path) {
 			FILE *file = fopen(real_file_path, "r");

 			char * res = read_file(file, real_file_path, "HEAD", 200);
 			write(socket, res, strlen(res));
 			fclose(file);
 			free(res);
 		}
 		else{

 			char *file_path = append_strings(conf->path, "/404.html");
 			FILE *file = fopen(file_path, "r");

 			if(file != NULL) {
 				char * res = read_file(file, file_path, "HEAD", 404);
 				write(socket, res, strlen(res));
 				fclose(file);
 				free(res);
 			}
 			free(file_path);
 			free(real_file_path);
 		}
 		free(str);
 	}	
 }

/*
 * Function that read the requested html page, 
 * builds a reponse and returns a pointer to
 * the response
 * @PARAM {FILE *file} The html page that the client has requested
 * @PARAM
 * @PARAM
 * @PARAM
 * @RETURN A pointer to the response
 */
char * read_file(FILE *file, char *file_path, char *method, int code) {

	char *response;
	int c;
	long file_size = -1;
	size_t n = 0;

	//Get size of file
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	response = build_headers(file_size, file_path, code);

	if(strcmp(method, "HEAD") == 0) {
		return response;
	}

	n = strlen(response);

	while((c = fgetc(file)) != EOF) {
		response[n++] = (char)c;
	}

	response[n] = '\0';

	return response;
}

/*
 * Reads the config file, looking for port,
 * concurrency method and path to base dir
 * @RETURN A conf variable that will be assigned
 * to the servers global conf variable
 */
Conf * read_conf(int daemon) {
	
	Conf * c = malloc(sizeof(Conf));
	c->port = -1;
	c->daemon = daemon;
	c->path = NULL,
	c->concurrency = NULL;
	FILE *file;
	char *buff;
	size_t file_size = 0;
	size_t index = 0;
	int c_har;
	char *token;

	file = fopen("./test.config", "rt");
	if(file != NULL) {	

		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		if(file_size == 0) {
			fclose(file);

			c->port = PORT;
			c->path = malloc(strlen(BASE_DIR));
			strncpy(c->path, BASE_DIR, strlen(BASE_DIR));
			c->concurrency = malloc(strlen(CONCURRENCY));
			strncpy(c->concurrency, CONCURRENCY, strlen(CONCURRENCY));

			if(c->daemon == -1) {
				printf("Server: No port specified, setting to system default: %d\n", PORT);
				printf("Server: No path specified, setting to system default: %s\n", BASE_DIR);
				printf("Server: No concurrency method specified, setting to system default: %s\n", CONCURRENCY);	
			}
		
			return c;
		}

		buff = malloc(file_size + 1);

		while((c_har = fgetc(file)) != EOF) {
			buff[index++] = (char) c_har;
		} 

		buff[index] = '\0';

		token = strtok(buff, "=");

		while(token) {

			if(strcmp(token, "DIR") == 0) {
				token = strtok(NULL, "\n");
				c->path = parse_dir(token);
				token = strtok(NULL, "=");
			}
			else if(strcmp(token, "PORT") == 0) {
				token = strtok(NULL, "\n");
				c->port = parse_port(token);
				
				if(c->port < 1024) {

					if(c->daemon == -1) {
						printf("Invalid port number found in config file, setting to system defulat: %d\n", PORT);
					}
					c->port = PORT;
				}

				token = strtok(NULL, "=");
			}
			else if(strcmp(token, "CON") == 0) {
				token = strtok(NULL, "\n");
				c->concurrency = parse_concurrency(token);
				token = strtok(NULL, "=");
			}
			else {
				if(c->daemon == -1) {
					printf("Format error in config file\n%s%s%s%s%s", 
						"Usage:\n",
						"\tDIR={your path}\n",
						"\tPORT={your port}\n",
						"\tCON={Your concurrency method}\n",
						"All are optional, if non are specified program argument or system default will be used\n");
				}
				exit(EXIT_FAILURE);
			}	
		}

		//No port was specified in the config file
		if(c->port == -1) {

			if(!c->daemon) {
				printf("No port number was specified in the config file, setting to system default: %d\n", PORT);
			}
			c->port = PORT;
		}

		//No path was specified in config, set to system default
		if(c->path == NULL) {
			if(c->daemon == -1) {
				printf("No path was specified in the config, setting to system defualt: %s\n", BASE_DIR);
			}
			c->path = malloc(strlen(BASE_DIR));
			strncpy(c->path, BASE_DIR, strlen(BASE_DIR));
		}

		//No concurrency method was specified in the config file
		if(c->concurrency == NULL) {
			if(c->daemon == -1) {
				printf("No concurrency method was specified in the config file, setting to system default: %s\n", CONCURRENCY);
			}
			c->concurrency = malloc(strlen(CONCURRENCY));
			strncpy(c->concurrency, CONCURRENCY, strlen(CONCURRENCY));
		}

		free(buff);
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
 * @PARAM {char * str} The string that holds the port number
 * @RETRUN The found port number
 */
int parse_port(char *str) {
	
	int port = -1;
	sscanf(str, "%d", &port);
	return port;
}

/*
 * A helper function to the parsing of the conf file
 * I looks for a path in a string and returns what it found
 * @PARAM {char * str} The string that holds the path
 * @RETRUN The found path
 */
char * parse_dir(char *str) {

	if(str != NULL) {
		char * r = malloc(strlen(str));
		strcpy(r, str);
		return r;
	}
	return NULL;
}

/*
 * A helper function to the parsing of the conf file
 * I looks for the concurrency in a string and returns what it found
 * @PARAM {char * str} The string that holds the path
 * @RETRUN The found path
 */
char * parse_concurrency(char *str) {

	if(str != NULL) {
		char * r = malloc(strlen(str));
		strcpy(r, str);
		return r;
	}
	return NULL;
}

/*
 * A helper function that appends two string
 * @PARAM {char *s1} First string
 * @PARAM {char *s2} Second string
 * @RETURN A pointer to the complete string
 */
char *append_strings(char *s1, char *s2) {

	size_t s1_len = strlen(s1);
	size_t s2_len = strlen(s2);

	char * r = malloc(1 + s1_len + s2_len);
	strcpy(r, s1);
	strcat(r, s2);
	r[s1_len+s2_len] = '\0';
	return r;
}

char *build_headers(long size, char *file_path, int code) {
	
	char *response;
	char *cont_len;
	size_t head_len = -1;
	char *last_mod;
	char *status_code;

	cont_len = cont_length(size);
	last_mod = mod_date(file_path);

	switch(code) {
		case 200:
			status_code = HTTP_OK;
			break;
		case 404:
			status_code = HTTP_NOT_FOUND;
			break;
		default:
			status_code = HTTP_OK;
			break;
	}

	head_len = strlen(status_code) +
		strlen(HEADER_CONT_TYPE) +
		strlen(HEADER_LANG) +
		strlen(cont_len) +
		strlen(last_mod) +
		strlen(HEADER_SERV_NAME) +
		strlen("\n");

	response = malloc(head_len + size + 1);

	strcpy(response, status_code);
	strcat(response, HEADER_LANG);
	strcat(response, HEADER_CONT_TYPE);
	strcat(response, cont_len);
	strcat(response, last_mod);
	strcat(response, HEADER_SERV_NAME);
	strcat(response, "\n");

	free(cont_len);
	free(last_mod);
	return response;
}

char * mod_date(char *path) {
	char date_arr[10];
	char *d_p;
	char *date;
	struct stat info;

	stat(path, &info);
    strftime(date_arr, 20, "%y/%m/%d/%H:%M:%S\n", localtime(&(info.st_ctime)));
    d_p = date_arr;
    date = append_strings(HEADER_LAST_MOD, d_p);

    return date;
}

char *cont_length(long size) {
	char body_len[10];
	sprintf(body_len, "%ld\n", size);
	char *b_l = body_len;
	char * cont_len = append_strings(HEADER_CONT_LEN, b_l);
	return cont_len;
}
