#include "../include/conf.h"

/*
 * Reads the config file, looking for port,
 * concurrency method and path to base dir
 * @PARAM {int daemon} -1 if not to run as daemon, 1 if to run as daemon
 * @RETURN A conf variable that will be assigned
 * to the servers global conf variable
 */
Conf * read_conf(int daemon) {
	
	Conf *c = malloc(sizeof(Conf));
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

			printf("Server: No port specified, setting to system default: %d\n", PORT);
			printf("Server: No path specified, setting to system default: %s\n", BASE_DIR);
			printf("Server: No concurrency method specified, setting to system default: %s\n", CONCURRENCY);	
			syslog(LOG_NOTICE, "Server: No port specified, setting to system default: %d\n", PORT);
			syslog(LOG_NOTICE, "Server: No path specified, setting to system default: %s\n", BASE_DIR);
			syslog(LOG_NOTICE, "Server: No concurrency method specified, setting to system default: %s\n", CONCURRENCY);
		
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
					printf("Invalid port number found in config file, setting to system defulat: %d\n", PORT);
					syslog(LOG_NOTICE, "Invalid port number found in config file, setting to system defulat: %d\n", PORT);
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
				_error(LOGFILE_FORMAT_ERROR);
			}	
		}

		//No port was specified in the config file
		if(c->port == -1) {
			printf("No port number was specified in the config file, setting to system default: %d\n", PORT);
			syslog(LOG_NOTICE, "No port number was specified in the config file, setting to system default: %d\n", PORT);
			c->port = PORT;
		}

		//No path was specified in config, set to system default
		if(c->path == NULL) {
			printf("No path was specified in the config, setting to system defualt: %s\n", BASE_DIR);
			syslog(LOG_NOTICE, "No path was specified in the config, setting to system defualt: %s\n", BASE_DIR);
			c->path = malloc(strlen(BASE_DIR));
			strncpy(c->path, BASE_DIR, strlen(BASE_DIR));
		}

		//No concurrency method was specified in the config file
		if(c->concurrency == NULL) {
			printf("No concurrency method was specified in the config file, setting to system default: %s\n", CONCURRENCY);
			syslog(LOG_NOTICE, "No concurrency method was specified in the config file, setting to system default: %s\n", CONCURRENCY);	
			c->concurrency = malloc(strlen(CONCURRENCY));
			strncpy(c->concurrency, CONCURRENCY, strlen(CONCURRENCY));
		}

		free(buff);
		fclose(file);
	}
	else {
		printf("ERROR");
		syslog(LOG_ERR, "Error, file not found");
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
		strncpy(r, str, strlen(str));
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
		strncpy(r, str, strlen(str));
		return r;
	}
	return NULL;
}