#include "response.h"

Response::Response(int status_code, 
                     string reason_phrase, 
                     string response_body,
                     string response_headers,
					 float http_version)
{
    this->http_version = http_version;
    this->status_code = status_code;

    this->reason_phrase = reason_phrase;
    this->response_headers = response_headers;
    this->response_body = response_body;
}
