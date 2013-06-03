#include "response.h"

namespace web
{
	Response::Response(int status_code,
					   std::string reason_phrase,
					   std::string response_body,
					   std::string response_headers,
					   float http_version)
	{
		this->http_version = http_version;
		this->status_code = status_code;

		this->reason_phrase = reason_phrase;
		this->response_headers = response_headers;
		this->response_body = response_body;
	}
}
