#include "web.h"

void log(int type, const char *s1, const char *s2, int num)
{
    int fd;
    char logbuffer[BUFSIZE * 2];

    switch (type)
    {
        case ERROR: 
            if(num != 0)
            {
                sprintf(logbuffer, "HTTP/1.1 500 Error\rnContent-Type: text/plain\r\nConnection: close\r\n\r\n"
                                   "SORRY\r\n%s\r\n%s\r\n", s1, s2);

                write(num, logbuffer, strlen(logbuffer));
            }

            sprintf(logbuffer, "ERROR: [%s]:[%s] Errno=(%d) exiting", s1, s2, errno); 

            break;

        case LOG: 
            (void)sprintf(logbuffer," INFO: [%s]:[%s]:%d",s1, s2,num); 
            break;
    }

    /* no checks here, nothing can be done a failure anyway */
    if((fd = open("logs/web.log", O_CREAT| O_WRONLY | O_APPEND, 0644)) >= 0)
    {
        write(fd, logbuffer, strlen(logbuffer));
        write(fd, "\n", 1);
        close(fd);
    }
    
    if(type == ERROR)
        exit(2);
}

CResponse request_handler(const char *resource_path, vector<CHeader *> headers, const char *verb)
{
    char response_headers[1000];
    const char *default_mimetype = "text/plain";
    sprintf(response_headers, "Content-Type: %s\r\n", default_mimetype);

    char response_body[5000];
    char header_line[1000];
    sprintf(response_body, "Request Path:\n\t%s\n\nHeaders:\n", resource_path);

    vector<CHeader *>::iterator header_it = headers.begin();

    int i = strlen(response_body);
    while(header_it != headers.end())
    {
        sprintf(header_line, "\t%s: %s\n", (*header_it)->get_name(), 
                                           (*header_it)->get_value());
 
        int header_len = strlen(header_line);
        strcpy(&response_body[i], header_line);
        response_body[i + header_len] = 0;

        ++header_it;
        i += header_len;
    }

    float http_version = 1.1;
    int status_code = 200;

    return CResponse(http_version, status_code, (char *)"OK", response_headers, 
                     response_body);
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Please provide a port number.\n\n");
        return 1;
    }

    int port = atoi(argv[1]);

    int result;
    if((result = run_server(request_handler, log, port)) != 0)
        return 2;
    
    return 0;
}

