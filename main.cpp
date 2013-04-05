#include "web.h"

void log(int type, const char *s1, const char *s2, int num)
{
        int fd;
        char logbuffer[BUFSIZE*2];

        switch (type) {
//        case ERROR: (void)sprintf(logbuffer,"ERROR: %s:%s Errno=%d exiting\npid=%d",s1, s2, errno,getpid()); break;
        case ERROR: (void)sprintf(logbuffer,"ERROR: %s:%s Errno=%d exiting",s1, s2, errno); break;
        case SORRY: 
                (void)sprintf(logbuffer, "<HTML><BODY><H1>nweb Web Server\nSorry: %s %s</H1></BODY></HTML>\r\n", s1, s2);
                
                (void)write(num,logbuffer,strlen(logbuffer));
                (void)sprintf(logbuffer,"SORRY: %s:%s",s1, s2); 
                break;
        case LOG: (void)sprintf(logbuffer," INFO: %s:%s:%d",s1, s2,num); 
           break;
        }
        /* no checks here, nothing can be done a failure anyway */
        if((fd = open("web.log", O_CREAT| O_WRONLY | O_APPEND, 0644)) >= 0)
        {
            (void)write(fd, logbuffer, strlen(logbuffer));
            (void)write(fd, "\n", 1);
            (void)close(fd);
        }
}

response_t request_handler(const char *resource_path, vector<header_t> headers, const char *verb)
{
    char response_headers[1000];
    const char *default_mimetype = "text/plain";
    sprintf(response_headers, "Content-Type: %s\r\n", default_mimetype);

    char response_body[5000];
    char header_line[1000];
    sprintf(response_body, "Request Path:\n\t%s\n\nHeaders:\n", resource_path);

    vector<header_t>::iterator header_it = headers.begin();

    int i = strlen(response_body);
    while(header_it != headers.end())
    {
        sprintf(header_line, "\t%s: %s\n", (*header_it).name, (*header_it).value);
 
        int header_len = strlen(header_line);
        strcpy(&response_body[i], header_line);
        response_body[i + header_len] = 0;

        ++header_it;
        i += header_len;
    }

    response_t response = { 0, 0, "", "", ""};

    response.http_version = 1.1;
    response.status_code = 200;
    strcpy(response.reason_phrase, "OK\0");
    strcpy(response.response_headers, response_headers);
    response.response_headers[strlen(response_headers)] = 0;

    strcpy(response.response_body, response_body);
    response.response_body[strlen(response_body)] = 0;

    return response;
}

int main(int argc, char **argv)
{
    run_server(request_handler, log, 5016);
    
    return 0;
}

