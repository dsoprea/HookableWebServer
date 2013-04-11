#include "cresponse.h"

CResponse::CResponse(float http_version, 
                     int status_code, 
                     char *reason_phrase, 
                     char *response_headers, 
                     char *response_body)
{
    this->http_version = http_version;
    this->status_code = status_code;

    this->reason_phrase = _strdup(reason_phrase);
    this->response_headers = _strdup(response_headers);
    this->response_body = _strdup(response_body);
}

CResponse::~CResponse()
{
    free(reason_phrase);
    free(response_headers);
    free(response_body);
}

