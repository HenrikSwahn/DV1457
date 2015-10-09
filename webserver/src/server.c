#include "../include/server.h"

Conf * conf;

//For ignoring signals, used when creating daemon
struct sigaction sa;

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

	closelog();
	exit(EXIT_SUCCESS);
}

/*
 * A function that is called if -d argument is sent to program
 */
void make_daemon() {

	char actual_path[PATH_MAX + 1];
	char *full_path = realpath(conf->path, actual_path);
	free(conf->path);

	conf->path = malloc(strlen(full_path));
	strncpy(conf->path, full_path, strlen(full_path));
	
	pid_t p_id = 0;
	p_id = fork();

	if(p_id < 0) {
		_error(FORK_ERROR);
	}

	if(p_id > 0) {
		exit(EXIT_SUCCESS);
	}

	pid_t sid;
	sid = setsid();
	if(sid < 0) {
	  _error(SESSION_ERROR);
	}

	sa.sa_handler = SIG_IGN;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGHUP, &sa ,NULL);

	p_id = fork();
	if(p_id < 0){
	  _error(FORK_ERROR);
	}
	if(p_id > 0){
	  printf("Daemon has process id: %d", p_id);
	  syslog(LOG_NOTICE, "Daemon has process id: %d", p_id);
	  exit(EXIT_SUCCESS);
	}
	
	umask(0); 
	syslog(LOG_NOTICE, "Successfully started daemon\n");

	if((chdir("/")) < 0) {
		_error(CHANGE_DIR_ERROR);
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
 * @PARAM {int daemon} -1 if not to run as daemon, 1 if to run as daemon
 */
void run_server(int lPort, int daemon) {
	
	signal(SIGINT, cleanup);
	struct sockaddr_in client;		
	int server_sock;
	int index;
	fd_set file_set;
	fd_set ready_files;
	socklen_t sock_len;
	openlog("Server" , LOG_NOWAIT | LOG_PID, LOG_USER);

	server_sock = create_server(lPort, daemon);

	if(conf->daemon == 1) {
		make_daemon();
	}
      
	if(listen(server_sock, MAXQ) < 0) {
		_error(SLISTEN_ERROR);
	}

	printf("%s%s\n", 
	"__________________________________________________________________________\n",
	"|______________Henrik_and_Andreas_webserver_is_firing up!!_______________|\n"); 
	printf("Server started listening on port: %d\nMain dir is: %s\nConcurrency method is set to: %s\n\n", conf->port, conf->path, conf->concurrency);
	printf("Wating for connection....\n");
	syslog(LOG_NOTICE, "Server started on port: %d\n", conf->port);
	syslog(LOG_NOTICE, "Main dir is: %s\n", conf->path);
	syslog(LOG_NOTICE, "Concurrency is set to: %s\n", conf->concurrency);
	
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

					
					printf("\t%s:%d connected.\n", inet_ntoa(client.sin_addr), (int)ntohs(client.sin_port));	
					syslog(LOG_NOTICE, "%s:%d connected.\n", inet_ntoa(client.sin_addr), (int)ntohs(client.sin_port));

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
	
	char *buffer = malloc(1024);
	int ret;
	
	ret = read(socket, buffer, 1024);
	
	if(ret < 0) {
		free(buffer);
		_error(READ_ERROR);
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
 * @PARAM {int daemon} -1 if not to run as daemon, 1 if to run as daemon
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
		printf("Server: Got port: %d from program argument, overriding system default/config file port\n", conf->port);
		syslog(LOG_NOTICE, "Got port: %d from program argument, overriding system default/config file port\n", conf->port);
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
 * @PARAM {char *buffer} A buffer with clients request
 */
void parse_request(int socket, char *buffer) {
	
	char *token = strtok(buffer, " ");
	while(token) {

		if(strcmp(token, "GET") == 0) {
			token = strtok(NULL, " ");
			if(token != NULL) {
				if(strstr(token, "/") != NULL) {
					get_req(conf->path,token, socket);
					break;
				}
			}
		}
		else if(strcmp(token, "HEAD") == 0) {
			token = strtok(NULL, " ");
			if(token != NULL) {
				if(strstr(token, "/") != NULL) {
					head_req(conf->path, token, socket);
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
