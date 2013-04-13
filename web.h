#ifndef __WEB_H
#define __WEB_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#include "socket_compat.h"
#include "thread_compat.h"

#include <vector>

using namespace std;

#include "cresponse.h"
#include "cheader.h"
#include "string_util.h"
#include "compat.h"

#define BUFSIZE 8096L

#define HNL "\r\n"
#define HNLL 2

#define LOG_RESPOND_ERROR(msg, fd) { if(callbacks.log_handler) callbacks.log_handler(log_type_e::T_ERROR, msg, "", fd); }
#define LOG_ERROR_SOCKET(msg, operation) { if(callbacks.log_handler) callbacks.log_handler(log_type_e::T_ERROR, msg, operation, 0); }
#define LOG_INFO(msg) { if(callbacks.log_handler) callbacks.log_handler(log_type_e::T_INFO, msg, "", 0); }

enum log_type_e { T_ERROR, T_INFO };

typedef void (*startup_handler_t)(int port);
typedef void (*shutdown_handler_t)();
typedef bool (*precycle_handler_t)();
typedef CResponse *(*request_handler_t)(const char *resource_path, vector<CHeader *> headers, const char *verb);
typedef void (*log_handler_t)(log_type_e type, const char *s1, const char *s2, int num);

typedef struct {
    // (optional) Triggered after the socket is configured.
    startup_handler_t startup_handler;
    
    // (optional) Triggered after requests are stopped, before the socket is 
    // closed.
    shutdown_handler_t shutdown_handler;
    
    // (optional) Receives logging verbosity.
    log_handler_t log_handler;

    // Called at the top of every loop. Returns FALSE to quit.
    precycle_handler_t precycle_handler;

    // Request handler.
    request_handler_t request_handler;
} server_callbacks_t;

typedef struct {
    int socketfd;
    server_callbacks_t callbacks;
} request_context_t;

int run_server(server_callbacks_t callbacks, int *port);

#endif

