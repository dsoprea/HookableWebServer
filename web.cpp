#include "web.h"

/* this is a child web server process, so we can exit on errors */
static int web(int fd, int hit, request_handler_t request_handler, log_handler_t log)
{
    int j, buflen, len;
    int file_fd;
    long i, ret;
    
    /* static so zero filled */
    char buffer[BUFSIZE + 1];
    char message[200];

    /* read Web request in one go */
    ret = read(fd, buffer, BUFSIZE);
    if(ret == 0 || ret == -1) 
        /* read failure stop now */
        BAIL("failed to read browser request.", fd);

    if(ret > 0 && ret < BUFSIZE)    
        /* return code is valid chars */
        buffer[ret] = 0;
    else
        /* terminate the buffer */
        buffer[0] = 0;

    // Iterate through the headers while we still have room for a two-
    // character newline, and we haven't encountered a double-newline.

    log(LOG, "Parsing request headers.", "", fd);

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
                BAIL("Could not allocate space for request-line.", fd);
            
            request_line_found = true;
        }
        else
        {
            char *text;
            if((text = _strdup(&buffer[i], found_at)) == NULL)
                BAIL("Could not allocate space for header.", fd);

            int colon_at;
            if((colon_at = _strpos(text, ":")) == -1)
                break;
                           
            char *name = _strdup(text, colon_at);
            char *value = _strdup(&text[colon_at + 2], (found_at - i) - (colon_at + 2));

            headers.push_back(new CHeader(name, value));
            
            //free(name);
            //free(value);

            free(text);
        }

        i += (found_at - i) + HNLL;
    }

    snprintf(message, 200, "Parsing request-line: %s", request_line);
    log(LOG, message, "", fd);

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
            strcpy(http_version, start_at);
            http_version[strlen(start_at)] = 0;
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
        BAIL("Request-line was not completely parsed.", fd);

    free(request_line);

    snprintf(message, 200, "Sending request with verb [%s], resource [%s], "
             "and version [%s] to handler.", verb, resource_path, 
             http_version);

    log(LOG, message, "", fd);

    CResponse response = request_handler(resource_path, headers, verb);

// TODO: Deallocate headers manually, here.

    vector<CHeader *>::iterator header_it = headers.begin();
    while(header_it != headers.end())
        delete (*header_it);

    log(LOG, "Building response.", "", fd);

    char version_string[9];
    snprintf(version_string, 9, "HTTP/%.1f", response.get_http_version());

    char status_code[4];
    snprintf(status_code, 4, "%d", response.get_status_code());

    int response_len = (strlen(version_string) + 1 + 
                        strlen(status_code) + 1 + 
                        strlen(response.get_reason_phrase()) + 2 + 
                        strlen(response.get_response_headers()) + 2 + 
                        strlen(response.get_response_body()));

    char *response_text;
    if((response_text = (char *)malloc(response_len)) == NULL)
    {
        BAIL("Could not allocate space for response.", fd);
        return -1;
    }

    snprintf(response_text, response_len, "%s %s %s\r\n%s\r\n%s", 
        version_string, status_code, response.get_reason_phrase(), 
        response.get_response_headers(), response.get_response_body());

    log(LOG, "Sending response.", "", fd);

    i = 0;
    int sent;
    buflen = strlen(response_text);
    int send_length;
    char chunk[BUFSIZE];
    while(i < buflen)
    {
        send_length = min(BUFSIZE, buflen - i);
        strncpy(chunk, &response_text[i], send_length);
        sent = write(fd, chunk, send_length);

        if(sent < BUFSIZE)
            break;

        i += BUFSIZE;
    }
    
    free(response_text);
    
    return 0;
}

int run_server(request_handler_t request_handler, log_handler_t log, int port)
{
        int i, pid, listenfd, socketfd, hit;
        socklen_t length;
        char *str;
        static struct sockaddr_in cli_addr; /* static = initialised to zeros */
        static struct sockaddr_in serv_addr; /* static = initialised to zeros */

        printf("Hosting at [http://localhost:%d].\n", port);

        /* Become deamon + unstopable and no zombies children 
           (= no wait()) */
/*
        if(fork() != 0)
                return 0; /* parent returns OK to shell /
*/
        (void)signal(SIGCLD, SIG_IGN); /* ignore child death */
        (void)signal(SIGHUP, SIG_IGN); /* ignore terminal hangups */

//        for(i=0;i<32;i++)
//                (void)close(i);      /* close open files /
/*
        (void)setpgrp();             /* break away from process group /

        log(LOG,"server starting",argv[1],getpid());
*/
        /* setup the network socket */

        if((listenfd = socket(AF_INET, SOCK_STREAM,0)) == -1)
        {
            BAILOP("system call","socket");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
        {
            BAILOP("system call","bind");
            return -2;
        }

        if(listen(listenfd,64) <0)
        {
            BAILOP("system call","listen");
            return -3;
        }

        hit = 0;
        while(1)
        {
            length = sizeof(cli_addr);
            if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
            {
                BAILOP("system call", "accept");
                return -4;
            }

            char message[100];
            sprintf(message, "Handling hit (%d).\n", hit);
            
            log(LOG, message, "", 0);

            if((pid = fork()) < 0)
            {
                BAILOP("system call", "fork");
                return -4;
            }
            else
            {
                if(pid == 0)
                {  
                    /* child */
                    (void)close(listenfd);
                    int result = web(socketfd, hit, request_handler, log);
                    
                    if(result != 0)
                    {
                        BAILOP("Server instance returned failure.", "web");
                        return -1;
                    }

                    return 0;
                } 
                else 
                {        
                    /* parent */
                    (void)close(socketfd);
                }
            }
            
            hit++;
        }
}

