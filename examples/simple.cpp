#include <iostream>

#include "../web.h"

static bool process_requests = true;

void startup_handler(void *server_info, int port)
{
    // Called at server startup.
    //
    // Parameters:
    //
    //   server_info: A ServerInfo instance that describes the callbacks, the
    //                  server port, request-data (general data meant to be sent
    //                to the request handler), and server-data (general server-
    //                  related data). Although you may use the request-data and
    //                  server-data for your own purposes (you must initialize it
    //                  and pass it into run_server, like below), the request-
    //                  data is used by the request-router for mapping info, and
    //                  will therefore be unavailable in that situation.
    //
    //   port: The actual port that the server bound to, whether it was
    //           explicit or random.
    //

    std::cout << "Web server starting on port (" << port << ")." << std::endl;
}

void shutdown_handler(void *server_info)
{
    // Called at server shutdown.

    std::cout << "Web server stopping." << std::endl;
}

bool continue_flag_handler(void *server_info)
{
    // Called frequently to determine if we should still be accepting requests.

    return process_requests;
}

web::Response request_handler(void *server_info, std::string resource_path,
                                 std::vector<web::Header> headers,
                                 std::string verb, std::string body)
{
    // Receives every request (when used the request-router, the router
    // receives all requests, and passes them to your "actions".

    std::string response_body = "Hello <b>there</b>. You requested "
                                "[" + resource_path + "].";

    web::Response response = web::get_general_response(200,
                                                         response_body,
                                                         "text/html");

    return response;
}

void log_handler(void *server_info, web::web_log_e type, const char *s1,
                 const char *s2, int num)
{
    // Receives logging information.

}

int main()
{
    web::ServerInfo server_info(startup_handler,
                                shutdown_handler,
                                log_handler,
                                continue_flag_handler,
                                request_handler,
                                0);

    // Run the server, and don't start it in a new thread.
    return web::run_server(&server_info, false, NULL);
}
