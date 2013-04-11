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

#include "cresponse.h"
#include "cheader.h"
#include "string_util.h"

#define BUFSIZE 8096L
#define ERROR 42
#define SERROR 43
#define LOG   44

#define HNL "\r\n"
#define HNLL 2

#define BAIL(msg, fd) { log(ERROR, msg, "", fd); exit(1); }
#define BAILOP(msg, op) { log(ERROR, msg, op, 0); exit(1); }

typedef CResponse (*request_handler_t)(const char *resource_path, vector<CHeader *> headers, const char *verb);
typedef void (*log_handler_t)(int type, const char *s1, const char *s2, int num);

int run_server(request_handler_t request_handler, log_handler_t log, int port);
//char *_strndup(char *buffer_old, int len);

#endif

