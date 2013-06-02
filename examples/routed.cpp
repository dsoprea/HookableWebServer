#include <iostream>

#include "../web.h"
#include "../request_router.h"

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

Response ah_hello(void *server_info, router::ActionDescriptor action,
		          string resource_path, map<string, string> parameters,
				  vector<Header> headers, router::request_verb_e verb,
				  string body)
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
	router::RouteMappings mappings;
	mappings.Add(1, "/hello", ah_hello);

	router::RequestRouterData request_router_data(&mappings);

	ServerInfo server_info(startup_handler,
						   shutdown_handler,
						   log_handler,
						   continue_flag_handler,
						   router::request_router,
						   11111,
						   &request_router_data);

	return run_server(&server_info, false, NULL);
}
