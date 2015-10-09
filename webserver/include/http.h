#ifndef HTTP_H
#define HTTP_H

#define HTTP_OK "HTTP/1.0 200 OK\n"
#define HTTP_BAD_REQUEST "HTTP/1.0 400 Bad Request\n"
#define HTTP_NOT_FOUND "HTTP/1.0 404 Not Found\n"
#define HTTP_INTERNAL_SERVER_ERROR "HTTP/1.0 500 Internal Server Error\n"
#define HTTP_NOT_IMPL "HTTP/1.0 501 Not Implemented\n"
#define HEADER_CONT_TYPE "Content-Type: text/html\n"
#define HEADER_LANG "Content-Language: en\n"
#define HEADER_CONT_LEN "Content-Length: "
#define HEADER_LAST_MOD "Last-Modified: "
#define HEADER_SERV_NAME "Server: Henrik/Andreas 1.0\n"

void http_200(int, char *, char *);
void http_400(int, char *);
void http_404(int, char *);
void http_500(int, char *);
void get_req(char *, int);
void head_req(char *, int);

#endif