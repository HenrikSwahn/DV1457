#include "../include/error.h"

void _error(int error) {

	switch(error) {
		case SACCEPT_ERROR:
			_socket_accept_error();
			break;
		case SLISTEN_ERROR:
			_socket_listen_error();
			break;
		case SCREATE_ERROR:
			_socket_create_error();
			break;
		case SBIND_ERROR:
			_socket_bind_error();
			break;
		case FORK_ERROR:
			_fork_error();
			break;
		case SESSION_ERROR:
			_session_error();
			break;
		case CHANGE_DIR_ERROR:
			_change_dir_error();
			break;
		case READ_ERROR:
			_read_error();
			break;
		case LOGFILE_FORMAT_ERROR:
			_logfile_format_error();
			break;
	}
}

void _socket_accept_error() {
	printf("Error accepting socket\n");
  	syslog(LOG_ERR, "Error accepting socket\n");
	exit(EXIT_FAILURE);
}

void _socket_bind_error() {
	printf("Error binding socket\n");
  	syslog(LOG_ERR, "Error binding socket\n");
	exit(EXIT_FAILURE);
} 

void _socket_listen_error() {
	printf("Socket listen error\n");
  	syslog(LOG_ERR, "Socket listen error\n");
	exit(EXIT_FAILURE);
}

void _socket_create_error() {
	printf("Error creating socket\n");
  	syslog(LOG_ERR, "Error creating socket\n");
	exit(EXIT_FAILURE);
}

void _select_error() {
	printf("Select error\n");
  	syslog(LOG_ERR, "Select error\n");
	exit(EXIT_FAILURE);
}

void _fork_error() {
	printf("Failed to make second fork\n");
  	syslog(LOG_ERR, "Failed to make second fork\n");
	exit(EXIT_FAILURE);
}

void _session_error() {
	printf("Could not create process group\n");
	syslog(LOG_ERR, "Could not create process group\n");
	exit(EXIT_FAILURE);
}

void _change_dir_error() {
	printf("Could not change dir\n");
	syslog(LOG_ERR, "Could not change dir\n");
	exit(EXIT_FAILURE);
}

void _read_error() {
	printf("Error reading input stream\n");
	syslog(LOG_ERR, "Error reading input stream\n");
	exit(EXIT_FAILURE);
}

void _logfile_format_error() {
	printf("Format error in config file\n%s%s%s%s%s", 
		"Usage:\n",
		"\tDIR={your path}\n",
		"\tPORT={your port}\n",
		"\tCON={Your concurrency method}\n",
		"All are optional, if non are specified program argument or system default will be used\n");
	exit(EXIT_FAILURE);
}