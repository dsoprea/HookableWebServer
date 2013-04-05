#ifndef __WEB_H
#define __WEB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <vector>

using namespace std;


#define BUFSIZE 8096L
#define ERROR 42
#define SORRY 43
#define LOG   44

#define HNL "\r\n"
#define HNLL 2

typedef struct 
{
    float http_version;
    int status_code;
    char reason_phrase[50];
    char response_headers[1000];
    char response_body[5000];
} response_t;

typedef struct
{
    char name[50];
    char value[500];
} header_t;

typedef response_t (*request_handler_t)(const char *resource_path, vector<header_t> headers, const char *verb);
typedef void (*log_handler_t)(int type, const char *s1, const char *s2, int num);

void run_server(request_handler_t request_handler, log_handler_t log, int port);
//char *_strndup(char *buffer_old, int len);

#endif

