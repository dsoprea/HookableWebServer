#ifndef __RESPONSE_H
#define __RESPONSE_H

#include <string>

#include <stdlib.h>

namespace web
{
	class Response
	{
		float http_version;
		int status_code;
		std::string reason_phrase;
		std::string response_headers;
		std::string response_body;

		public:
			Response(int status_code,
					 std::string reason_phrase,
					 std::string response_body,
					 std::string response_headers="",
					 float http_version=1.1f);

			float get_http_version() { return http_version; }
			int get_status_code() { return status_code; }
			std::string get_reason_phrase() { return reason_phrase; }
			std::string get_response_headers() { return response_headers; }
			std::string get_response_body() { return response_body; }
	};
}

#endif
