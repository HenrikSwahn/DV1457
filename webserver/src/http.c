#include "../include/http.c"

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

 	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;

	//URL not ok
	if(check_url(path) == -1) {
		http_400(socket, "GET");
	}
	else { //URL OK

	 	if(strcmp(path, "/") == 0) {
	 		str = append_strings(conf->path, "/index.html");
	 	}
	 	else {
	 		str = append_strings(conf->path, path); 
	 	}
	 	free(str);
	 	real_file_path = realpath(str,actualPath);

	 	if(real_file_path) {
	 		http_200(socket, real_file_path, "GET");
	 	}
	 	else {
	 		free(real_file_path);
	 		http_404(socket, "GET");
	 	}
	}
}

/*
 * Function to handle if the client made a HEAD request.
 * @PARAM {char *path} The path the client want to acces
 * @PARAM {int socket} server socket filedescriptor value
 */
 void head_req(char *path, int socket) {

 	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;

	//URL not ok
	if(check_url(path) == -1) {
		http_400(socket, "HEAD");
	}
	else { //URL OK

	 	if(strcmp(path, "/") == 0) {
	 		str = append_strings(conf->path, "/index.html");
	 	}
	 	else {
	 		str = append_strings(conf->path, path); 
	 	}
	 	free(str);
	 	real_file_path = realpath(str,actualPath);

	 	if(real_file_path) {
	 		http_200(socket, real_file_path, "HEAD");
	 	}
	 	else {
	 		free(real_file_path);
	 		http_404(socket, "HEAD");
	 	}
	}	
 }

void http_200(int socket, char *file_path, char *method) {
	char *res;
	FILE *file;

	file = fopen(file_path, "r");

	if(file != NULL) {
		res = read_file(file, file_path, method, 200);
		write(socket, res, strlen(res));
		fclose(file);
		free(res);
	}
	else {
		http_500(socket, method);
	}
}

void http_400(int socket, char *method) {

	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;
	char *res;
	FILE *file;

	str = append_strings(conf->path, "/400.html");
	real_file_path = realpath(str, actualPath);
	free(str);

	if(real_file_path) {
		file = fopen(real_file_path, "r");

		if(file != NULL) {
			res = read_file(file, real_file_path, method, 400);
			write(socket, res, strlen(res));
			fclose(file);
			free(res);
		}
		else {
			http_500(socket, method);
		}
	}
	else {
		free(real_file_path);
	}
}

void http_404(int socket, char *method) {
	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;
	char *res;
	FILE *file;

	str = append_strings(conf->path, "/404.html");
	real_file_path = realpath(str, actualPath);
	free(str);

	if(real_file_path) {
		file = fopen(real_file_path, "r");

		if(file != NULL) {
			res = read_file(file, real_file_path, method, 404);
			write(socket, res, strlen(res));
			fclose(file);
			free(res);
		}
		else {
			http_500(socket, method);
		}
	}
	else {
		free(real_file_path);
	}
}

void http_500(int socket, char *method) {

	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;
	char *res;
	FILE *file;

	str = append_strings(conf->path, "/500.html");
	real_file_path = realpath(str, actualPath);
	free(str);

	if(real_file_path) {
		file = fopen(real_file_path, "r");

		if(file != NULL) {
			res = read_file(file, real_file_path, method, 500);
			write(socket, res, strlen(res));
			fclose(file);
			free(res);
		}
	}
	else {
		free(real_file_path);
	}
}