#ifndef __CRESPONSE_H
#define __CRESPONSE_H

#include <stdlib.h>
#include <string.h>
#include "string_util.h"

class CResponse
{
    float http_version;
    int status_code;
    char *reason_phrase;
    char *response_headers;
    char *response_body;

    public:
        CResponse(float http_version, 
                  int status_code, 
                  char *reason_phrase, 
                  char *response_headers, 
                  char *response_body);
        ~CResponse(); 
    
        float get_http_version() { return http_version; }
        int get_status_code() { return status_code; }
        char *get_reason_phrase() { return reason_phrase; }
        char *get_response_headers() { return response_headers; }
        char *get_response_body() { return response_body; }
};

#endif

