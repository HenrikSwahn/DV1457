#include "../include/http.h"

/*
 * Function to handle if the client made a GET request.
 * It first checks if the client want to acces /,
 * if so it sends that file if it can be found. If the client
 * want to access something els like "/index.html" the function 
 * tries to open that file, if fail returns not found to client
 * @PARAM {char *path} The path the client want to acces
 * @PARAM {int socket} server socket filedescriptor value
 */
 void get_req(char *conf_path, char *path, int socket) {

 	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;

	//URL not ok
	if(check_url(path) == -1) {
		http_400(conf_path, socket, "GET");
	}
	else { //URL OK

	 	if(strcmp(path, "/") == 0) {
	 		str = append_strings(conf_path, "/index.html");
	 	}
	 	else {
	 		str = append_strings(conf_path, path); 
	 	}
	 	free(str);
	 	real_file_path = realpath(str,actualPath);

	 	if(real_file_path) {
	 		http_200(socket, real_file_path, "GET");
	 	}
	 	else {
	 		free(real_file_path);
	 		http_404(conf_path, socket, "GET");
	 	}
	}
}

/*
 * Function to handle if the client made a HEAD request.
 * @PARAM {char *path} The path the client want to acces
 * @PARAM {int socket} server socket filedescriptor value
 */
 void head_req(char *conf_path, char *path, int socket) {

 	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;

	//URL not ok
	if(check_url(path) == -1) {
		http_400(conf_path, socket, "HEAD");
	}
	else { //URL OK

	 	if(strcmp(path, "/") == 0) {
	 		str = append_strings(conf_path, "/index.html");
	 	}
	 	else {
	 		str = append_strings(conf_path, path); 
	 	}
	 	free(str);
	 	real_file_path = realpath(str,actualPath);

	 	if(real_file_path) {
	 		http_200(socket, real_file_path, "HEAD");
	 	}
	 	else {
	 		free(real_file_path);
	 		http_404(conf_path, socket, "HEAD");
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
		http_500(NULL, socket, method);
	}
}

void http_400(char *conf_path, int socket, char *method) {

	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;
	char *res;
	FILE *file;

	str = append_strings(conf_path, "/400.html");
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
			http_500(conf_path, socket, method);
		}
	}
	else {
		free(real_file_path);
		http_500(conf_path, socket, method);
	}
}

void http_403(char *conf_path, int socket, char *method) {

	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;
	char *res;
	FILE *file;

	str = append_strings(conf_path, "/403.html");
	real_file_path = realpath(str, actualPath);
	free(str);

	if(real_file_path) {
		file = fopen(real_file_path, "r");

		if(file != NULL) {
			res = read_file(file, real_file_path, method, 403);
			write(socket, res, strlen(res));
			fclose(file);
			free(res);
		}
		else {
			http_500(conf_path, socket, method);
		}
	}
	else {
		free(real_file_path);
		http_500(conf_path, socket, method);
	}
}

void http_404(char *conf_path, int socket, char *method) {
	char actualPath [PATH_MAX];
	char *str; 
	char *real_file_path;
	char *res;
	FILE *file;

	str = append_strings(conf_path, "/404.html");
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
			http_500(conf_path, socket, method);
		}
	}
	else {
		free(real_file_path);
		http_500(conf_path, socket, method);
	}
}

void http_500(char *conf_path, int socket, char *method) {

	char * res;
	size_t body_len = strlen("\n<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"><html><head><title>500</title></head><body><p>500 INTERNAL SERVER ERROR</p></body></html>\n");
	char * _500 = malloc(body_len);
	strcpy(_500, "\n<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"><html><head><title>500</title></head><body><p>500 INTERNAL SERVER ERROR</p></body></html>\n");
	res = malloc(strlen(_500) + strlen(HTTP_INTERNAL_SERVER_ERROR));

	strcpy(res, HTTP_INTERNAL_SERVER_ERROR);
	strcat(res, _500);

	write(socket, res, strlen(res));

	free(res);
	free(_500);
}

/*
 * Function that read the requested html page, 
 * builds a reponse and returns a pointer to
 * the response
 * @PARAM {FILE *file} The html page that the client has requested
 * @PARAM {char *file_path} The path to the file to be read
 * @PARAM {char *method} The http method to handle
 * @PARAM {int code} The http status
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
 * Function used to build the response headers
 * @PARAM {long size} The size of the response body
 * @PARAM {char *file_path} The filepath to file read
 * @PARAM {int code} The status code
 * @RETURN A pointer to the response string
 */
char * build_headers(long size, char *file_path, int code) {
	
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
		case 400:
			status_code = HTTP_BAD_REQUEST;
			break;
		case 404:
			status_code = HTTP_NOT_FOUND;
			break;
		case 500:
			status_code = HTTP_INTERNAL_SERVER_ERROR;
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

	strncpy(response, status_code, strlen(status_code));
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

/*
 * A function to get the last modfied for the file
 * @PARAM {char *path} The file path to the file 
 * that we want to check when last modfied 
 * @RETURN A pointer to the string containing the last mod header
 */
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

/*
 * A function to get the content length as a string
 * @PARAM {long size} The size of the body 
 * @RETURN A pointer to the string containing the content length header
 */
char * cont_length(long size) {
	char body_len[10];
	sprintf(body_len, "%ld\n", size);
	char *b_l = body_len;
	char *cont_len = append_strings(HEADER_CONT_LEN, b_l);
	return cont_len;
}

/*
 * A helper function that appends two string
 * @PARAM {char *s1} First string
 * @PARAM {char *s2} Second string
 * @RETURN A pointer to the complete string
 */
char * append_strings(char *s1, char *s2) {

	size_t s1_len = strlen(s1);
	size_t s2_len = strlen(s2);

	char * r = malloc(1 + s1_len + s2_len);
	strcpy(r, s1);
	strcat(r, s2);
	r[s1_len+s2_len] = '\0';
	return r;
}

int check_url(char * url) {

	char * i = url;
	char prev = *i;
	char t[] = "/.";

	*i++;

	while(*i) { //Loop through the charecters
		if(strchr(t, prev) != NULL) {
			if(*i == prev) {
				return -1;
			}
		}
		prev = *i;
		*i++;
	}
	return 1;
}