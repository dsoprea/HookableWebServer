#include "web.h"

static bool error_flag = false;
static bool do_requests_flag = true;

void log(log_type_e type, const char *s1, const char *s2, int num)
{
    FILE *fd;
    char logbuffer[BUFSIZE * 2];

    switch (type)
    {
        case log_type_e::T_ERROR: 
            do_requests_flag = true;

            if(num != 0)
            {
                sprintf(logbuffer, "HTTP/1.1 500 Error\rnContent-Type: text/plain\r\nConnection: close\r\n\r\n"
                                   "SORRY\r\n%s\r\n%s\r\n", s1, s2);

                SOCKET_WRITE(num, logbuffer, strlen(logbuffer));
            }

            sprintf(logbuffer, "ERROR: [%s]:[%s] Errno=(%d) exiting", s1, s2, errno); 
            break;

        case log_type_e::T_INFO:
            sprintf(logbuffer, "INFO: [%s]:[%s]:%d", s1, s2, num); 
            break;
    }
/*
    if((fd = fopen("logs/web.log", "a+")) != NULL)
    {
        fwrite(logbuffer, 1, strlen(logbuffer), fd);
        fwrite("\n", 1, 1, fd);
        fclose(fd);
    }
*/

    printf("%s\n", logbuffer);

    if(type == ERROR)
        do_requests_flag = true;
}

CResponse *request_handler(const char *resource_path, vector<CHeader *> headers, const char *verb)
{
    char response_headers[1000];
    const char *default_mimetype = "text/plain";
    int count;
    
    count = SNPRINTF(response_headers, 1000, "Content-Type: %s\r\n", default_mimetype);
    response_headers[count] = 0; // For windows.

    char response_body[5000];
    count = SNPRINTF(response_body, 5000,  "Request Path:\n\t%s\n\nHeaders:\n", resource_path);
    response_body[count] = 0; // For windows.

    char header_line[1000];
    vector<CHeader *>::iterator header_it = headers.begin();

    while(header_it != headers.end())
    {
        count = SNPRINTF(header_line, 1000, "\t%s: %s\n", (*header_it)->get_name(), (*header_it)->get_value());
        header_line[count] = 0; // For windows.

        int header_len = strlen(header_line);
        strcat(response_body, header_line);

        ++header_it;
    }

    float http_version = 1.1;
    int status_code = 200;

    return new CResponse(http_version, status_code, (char *)"OK", 
                         response_headers, response_body);
}

void startup_handler(int port)
{
    printf("Hosting at [http://localhost:%d].\n", port);
}

void shutdown_handler()
{

}

bool precycle_handler()
{
    printf("\nWaiting for request.\n\n");

    return do_requests_flag;
}

#ifdef _WIN32
BOOL WINAPI break_handler(DWORD ctrlType)
{
// TODO: Rewrite the accept() call to timeout and loop, so we can check for 
//       quit events (incoming requests will just have to wait for a couple of 
//       nanoseconds longer to be accepted, when that happens).
    // Received CTRL+BREAK. Handle the next request, and then terminate.
    if(ctrlType == CTRL_C_EVENT || ctrlType == CTRL_BREAK_EVENT)
    {
        do_requests_flag = false;
        return true;
    }
}
#endif

int main(int argc, char **argv)
{
    int result;
    int port = 0;

#ifdef _WIN32
    SetConsoleCtrlHandler(break_handler, true);
#endif

    server_callbacks_t callbacks = { 
        startup_handler, 
        shutdown_handler, 
        log,
        precycle_handler,
        request_handler
    };

    if((result = run_server(callbacks, &port)) != 0)
        return 1;
    else if(error_flag)
        return 2;
    
    return 0;
}

