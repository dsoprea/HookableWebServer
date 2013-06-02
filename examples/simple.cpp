#include <iostream>

#include "../web.h"

using namespace std;

static bool process_requests = true;

void startup_handler(void *server_info, int port)
{
	cout << "Web server starting on port (" << port << ")." << endl;
}

void shutdown_handler(void *server_info)
{
	cout << "Web server stopping." << endl;
}

bool continue_flag_handler(void *server_info)
{
	return process_requests;
}

Response request_handler(void *server_info, string resource_path,
						  vector<Header> headers, string verb, string body)
{
	Response response = get_general_response(200,
		            	  	      	         "Hello <b>there</b>. You "
									         	 "requested [" +
									         	 resource_path + "].",
									         "text/html");

	// This causes a race condition between termination and returning the
	// request, and may make the program terminate before the first/only
	// request responds.
    //process_requests = false;

	return response;
}

void log_handler(void *server_info, web_log_e type, const char *s1,
				 const char *s2, int num)
{

}

int main()
{
	ServerInfo server_info(startup_handler,
						   shutdown_handler,
						   log_handler,
						   continue_flag_handler,
						   request_handler,
						   11111);

	return run_server(&server_info, false, NULL);
}
