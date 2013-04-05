#include "web.h"
/*
char *_strndup(char *buffer_old, int len)
{
    char *buffer_new = (char *)malloc(len + 1);
    memcpy(buffer_new, buffer_old, len);
    buffer_new[len] = 0;

    return buffer_new;
}
*/

/* this is a child web server process, so we can exit on errors */
static void web(int fd, int hit, request_handler_t request_handler, log_handler_t log)
{
        int j, buflen, len;
        int file_fd;
        long i, ret;
        char buffer[BUFSIZE+1]; /* static so zero filled */

        ret = read(fd, buffer, BUFSIZE);   /* read Web request in one go */
        if(ret == 0 || ret == -1) {     /* read failure stop now */
             log(SORRY,"failed to read browser request","",fd);
             exit(3);
        }

        if(ret > 0 && ret < BUFSIZE)    /* return code is valid chars */
            buffer[ret]=0;          /* terminate the buffer */
        else 
            buffer[0]=0;

        // Loop over the content while we still have room for a two-character 
        // newline, and we haven't encountered a double-newline.

        char request_line[1000];
        bool request_line_found = false;
        int found_at;
        char *found_at_raw;
        char *copy_to;
        vector<header_t> headers;
        bool is_first_header = true;

        i = 0;
        while((found_at_raw = strstr(&buffer[i], HNL)) != NULL)
        {
            found_at = (found_at_raw - &buffer[i]);
            if(found_at == 0)
                break;
 
            char text[1000];
            strncpy(text, &buffer[i], found_at);
            text[found_at] = 0;

            if(request_line_found == false)
            {
                strcpy(request_line, text);
                request_line[strlen(text)] = 0;
                
                request_line_found = true;
            }
            else
            {
                char *colon_at_raw = strstr(text, ":");
                
                if(colon_at_raw == NULL)
                    break;
                               
                int colon_at = (int)(colon_at_raw - text);

                header_t header;
    
                strncpy(header.name, text, colon_at);
                header.name[colon_at] = 0;
                
                int value_len = found_at - (colon_at + 2);
                strncpy(header.value, &text[colon_at + 2], value_len);
                header.value[value_len] = 0;

                headers.push_back(header);
            }

            i += found_at + HNLL;
        }

        i = 0;
        int found_state = 0;
        char verb[10];
        char resource_path[255];
        char http_version[5];
        while(found_state < 3)
        {
            char *start_at = &request_line[i];

            char *found_at_raw = strstr(start_at, " ");
            if(found_at_raw == NULL)
            {
                strcpy(http_version, start_at);
                http_version[strlen(start_at)] = 0;
                break;
            }

            int found_at = (int)(found_at_raw - start_at);

            char component[200];
            strncpy(component, &request_line[i], found_at);
            component[found_at] = 0;

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

            i += comp_len + 1;
            found_state++;
        }

        response_t response = request_handler(resource_path, headers, verb);
        char response_text[2000];
        sprintf(response_text, "HTTP/%.1f %d %s\r\n%s\r\n%s", 
            response.http_version, response.status_code, 
            response.reason_phrase, 
            response.response_headers, 
            response.response_body);

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
}

void run_server(request_handler_t request_handler, log_handler_t log, int port)
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

        for(i=0;i<32;i++)
                (void)close(i);      /* close open files /
/*
        (void)setpgrp();             /* break away from process group /

        log(LOG,"server starting",argv[1],getpid());
*/
        /* setup the network socket */

        if((listenfd = socket(AF_INET, SOCK_STREAM,0)) == -1)
        {
            log(ERROR, "system call","socket",0);
            exit(3);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
        {
            log(ERROR,"system call","bind",0);
            exit(3);
        }

        if( listen(listenfd,64) <0)
        {
            log(ERROR,"system call","listen",0);
            exit(3);
        }

        for(hit=1; ;hit++) 
        {
            length = sizeof(cli_addr);
            if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
            {
                log(ERROR,"system call","accept",0);
                exit(3);
            }

            char message[200];
            sprintf(message, "Handling hit (%d).\n", hit);
            
            log(LOG, message, "", 0);

            if((pid = fork()) < 0) {
                    log(ERROR,"system call","fork",0);
                    exit(3);
            }
            else {
                    if(pid == 0) {  /* child */
                            (void)close(listenfd);
                            web(socketfd, hit, request_handler, log); /* never returns */
                    } else {        /* parent */
                            (void)close(socketfd);
                    }
            }
        }
}
