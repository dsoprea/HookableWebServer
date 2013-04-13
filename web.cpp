#include "web.h"

using namespace threading;

static int web(int fd, server_callbacks_t callbacks)
{
    int j, buflen, len, file_fd, count;
    long i, ret;

    /* static so zero filled */
    char buffer[BUFSIZE + 1];
    char message[200];

    /* read Web request in one go */
    ret = SOCKET_READ(fd, buffer, BUFSIZE);
    if(ret == 0 || ret == -1) 
    {
        /* read failure stop now */
        callbacks.log_handler(log_type_e::T_ERROR, "failed to read browser request.", "", fd);
        return -1;
    }

    if(ret > 0 && ret < BUFSIZE)    
        /* return code is valid chars */
        buffer[ret] = 0;
    else
        /* terminate the buffer */
        buffer[0] = 0;

    // Iterate through the headers while we still have room for a two-
    // character newline, and we haven't encountered a double-newline.

    LOG_INFO("Parsing request headers.");

    char *request_line;
    bool request_line_found = false;
    int found_at;
    char *copy_to;
    vector<CHeader *> headers;
    bool is_first_header = true;

    i = 0;
    while((found_at = _strpos(buffer, HNL, i)) != -1 && found_at != 0)
    {
        if(request_line_found == false)
        {
            if((request_line = _strdup(&buffer[i], found_at)) == NULL)
            {
                LOG_RESPOND_ERROR("Could not allocate space for request-line.", fd);
                return -2;
            }
            
            request_line_found = true;
        }
        else
        {
            char *text;
            if((text = _strdup(&buffer[i], found_at)) == NULL)
            {
                LOG_RESPOND_ERROR("Could not allocate space for header.", fd);
                return -3;
            }

            int colon_at;
            if((colon_at = _strpos(text, ":")) == -1)
                break;
                           
            char *name = _strdup(text, colon_at);
            char *value = _strdup(&text[colon_at + 2], (found_at - i) - (colon_at + 2));

            headers.push_back(new CHeader(name, value));
            
            free(name);
            free(value);

            free(text);
        }

        i += (found_at - i) + HNLL;
    }

    if(request_line_found == false)
        return -7;

    count = SNPRINTF(message, 200, "Parsing request-line: %s", request_line);
    message[count] = 0; // For windows.

    LOG_INFO(message);

    i = 0;
    int found_state = 0;
    char verb[10] = "";
    char resource_path[255] = "";
    char http_version[5] = "";
    while(found_state < 3)
    {
        char *start_at = &request_line[i];

        int found_at;
        if((found_at = _strpos(start_at, " ")) == -1)
        {
            if(strncmp(start_at, "HTTP/", 5) != 0)
                return -6;

            strcpy(http_version, &start_at[5]);
            break;
        }

        char *component;
        component = _strdup(&request_line[i], found_at);

        int comp_len = strlen(component);

        if(found_state == 0)
        {
            strcpy(verb, component);
            verb[comp_len] = 0;
        }
        else if(found_state == 1)
        {
            strcpy(resource_path, component);
            resource_path[comp_len] = 0;
        }

        free(component);

        i += comp_len + 1;
        found_state++;
    }

    if(strcmp(verb, "") == 0 || strcmp(resource_path, "") == 0 || 
       strcmp(http_version, "") == 0)
    {
        LOG_RESPOND_ERROR("Request-line was not completely parsed.", fd);
        return -4;
    }

    free(request_line);

    count = SNPRINTF(message, 200, "Sending request with verb [%s], resource [%s], "
             "and version [%s] to handler.", verb, resource_path, 
             http_version);
    message[count] = 0; // For windows.

    LOG_INFO(message);

    CResponse *response = callbacks.request_handler(resource_path, headers, verb);

    while(headers.empty() == false)
    {
        const int num_headers = headers.size();
        delete (headers[num_headers - 1]);

        headers.resize(num_headers - 1);
    }

    LOG_INFO("Building response.");

    char version_string[9];
    count = SNPRINTF(version_string, 9, "HTTP/%.1f", response->get_http_version());
    version_string[count] = 0; // For windows.

    char status_code[4];
    count = SNPRINTF(status_code, 4, "%d", response->get_status_code());
    status_code[count] = 0; // For windows.

    const int version_string_len = strlen(version_string);
    const int status_code_len = strlen(status_code);
    const int reason_phrase_len = strlen(response->get_reason_phrase());
    const int response_headers_len = strlen(response->get_response_headers());
    const int response_body_len = strlen(response->get_response_body());

    int response_len = (version_string_len + 1 +
                        status_code_len + 1 +
                        reason_phrase_len + 2 + 
                        response_headers_len + 2 + 
                        response_body_len) + 1;

    char *response_text;
    if((response_text = (char *)malloc(response_len)) == NULL)
    {
        LOG_RESPOND_ERROR("Could not allocate space for response.", fd);
        return -5;
    }

    count = SNPRINTF(response_text, response_len, "%s %s %s\r\n%s\r\n%s", 
             version_string, status_code, response->get_reason_phrase(), 
             response->get_response_headers(), response->get_response_body());
    response_text[count] = 0;

    delete response;

    LOG_INFO("Sending response.");

    i = 0;
    int sent;
    buflen = strlen(response_text);
    int send_length;
    char chunk[BUFSIZE];
    while(i < buflen)
    {
        send_length = min(BUFSIZE, buflen - i);
        strncpy(chunk, &response_text[i], send_length);
        sent = SOCKET_WRITE(fd, chunk, send_length);

        if(sent < BUFSIZE)
            break;

        i += BUFSIZE;
    }
    
    free(response_text);

    return 0;
}

bool handle_request(void *thread_info_raw)
{
    request_context_t *request_context = (request_context_t *)thread_info_raw;

    int socketfd = request_context->socketfd;
    server_callbacks_t callbacks = request_context->callbacks;

    int result = web(socketfd, callbacks);

    // This was opened when we received this particular request, before the 
    // thread spawned.
    SOCKET_CLOSE(socketfd);

    // This was allocated just before our thread was spawned. Free it now that 
    // the thread is done.
    free(request_context);

    if(result != 0)
    {
        LOG_ERROR_SOCKET("Server instance returned failure.", "web");
        return false;
    }

    return true;
}

int run_server(server_callbacks_t callbacks, int *port)
{
        int i, pid, listenfd, socketfd, hit;
        socklen_t length;
        char *str;
        static struct sockaddr_in cli_addr; /* static = initialised to zeros */
        static struct sockaddr_in serv_addr; /* static = initialised to zeros */

        SOCKET_STARTUP();

        /* setup the network socket */

        if((listenfd = SOCKET(AF_INET, SOCK_STREAM,0)) == -1)
        {
            LOG_ERROR_SOCKET("system call", "socket");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");// htonl(INADDR_ANY);
        serv_addr.sin_port = htons(*port);
        
        if(SOCKET_BIND(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0)
        {
            LOG_ERROR_SOCKET("system call", "bind");
            return -2;
        }

        if(SOCKET_LISTEN(listenfd, 64) <0)
        {
            LOG_ERROR_SOCKET("system call", "listen");
            return -3;
        }

        if(*port == 0)
        {
            int addrlen = sizeof(serv_addr);
            if(SOCKET_NAME(listenfd, (struct sockaddr *)&serv_addr, &addrlen) != 0)
            {
                LOG_ERROR_SOCKET("system call", "getsockname");
                return -4;
            }

            *port = ntohs(serv_addr.sin_port);
        }

        if(callbacks.startup_handler)
            callbacks.startup_handler(*port);

        while(callbacks.precycle_handler())
        {
            length = sizeof(cli_addr);
            if((socketfd = SOCKET_ACCEPT(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
            {
                LOG_ERROR_SOCKET("system call", "accept");
                return -5;
            }

//            char message[100];
//            sprintf(message, "Handling hit (%d).\n", hit);
//            LOG_INFO(message);

/*            if((pid = fork()) < 0)
            {
                BAILOP("system call", "fork");
                return -6;
            }
            else
            {
                if(pid == 0)
                {  
                    /* child * /
*/
//                    SOCKET_CLOSE(listenfd);

                    request_context_t *request_context = (request_context_t *)malloc(sizeof(request_context_t));
                    request_context->socketfd = socketfd;
                    request_context->callbacks = callbacks;
                    
                    if(ThreadWrapper(handle_request, request_context).Start() == false)
                        break;
                    
//                    return 0;
/*                } 
                else 
                {        
*/
                /* parent */
//                    SOCKET_CLOSE(socketfd);
/*
                    }
            }
*/
        }

        SOCKET_CLOSE(listenfd);
                    
        if(callbacks.shutdown_handler)
            callbacks.shutdown_handler();

        SOCKET_SHUTDOWN();
}

