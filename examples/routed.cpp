#include <iostream>

#include "../web.h"
#include "../request_router.h"

static bool process_requests = true;

void startup_handler(void *server_info, int port)
{
	std::cout << "Web server starting on port (" << port << ")." << std::endl;
}

void shutdown_handler(void *server_info)
{
	std::cout << "Web server stopping." << std::endl;
}

bool continue_flag_handler(void *server_info)
{
	return process_requests;
}

web::Response ah_hello(void *server_info, router::ActionDescriptor action,
		          std::string resource_path, std::map<std::string,
		          std::string> parameters, std::vector<web::Header> headers,
		          router::request_verb_e verb, std::string body)
{
	std::string response_body = "Hello <b>there</b>. You requested "
								"[" + resource_path + "].";

	web::Response response = web::get_general_response(200,
		            	  	      	         	 	   response_body,
		            	  	      	         	 	   "text/html");

	// This causes a race condition between termination and returning the
	// request, and may make the program terminate before the first/only
	// request responds.
    //process_requests = false;

	return response;
}

void log_handler(void *server_info, web::web_log_e type, const char *s1,
				 const char *s2, int num)
{

}

int main()
{
	router::RouteMappings mappings;
	mappings.Add(1, "/hello", ah_hello);

	router::RequestRouterData request_router_data(&mappings);

	web::ServerInfo server_info(startup_handler,
						   	    shutdown_handler,
						   	    log_handler,
						   	    continue_flag_handler,
						   	    router::request_router,
						   	    0,
						   	    &request_router_data);

	return run_server(&server_info, false, NULL);
}
