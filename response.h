#ifndef __RESPONSE_H
#define __RESPONSE_H

#include <string>

#include <stdlib.h>

using namespace std;

class Response
{
    float http_version;
    int status_code;
    string reason_phrase;
    string response_headers;
    string response_body;

    public:
        Response(int status_code,
			      string reason_phrase, 
                  string response_body,
                  string response_headers="",
				  float http_version=1.1f);
                  
        float get_http_version() { return http_version; }
        int get_status_code() { return status_code; }
		string get_reason_phrase() { return reason_phrase; }
        string get_response_headers() { return response_headers; }
        string get_response_body() { return response_body; }
};

#endif

