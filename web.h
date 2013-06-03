#ifndef __WEB_H
#define __WEB_H

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "support/thread.h"
#include "response.h"
#include "header.h"

#define BUFSIZE 8096L

#define HNL "\r\n"
#define HNLL 2

#define LOG_RESPOND_ERROR(msg, fd) { \
	if(server_info->get_log_handler()) \
		server_info->get_log_handler()(server_info, LT_ERROR, msg, "", fd); \
}

#define LOG_ERROR_SOCKET(msg, operation) { \
	if(server_info->get_log_handler()) \
		server_info->get_log_handler()(server_info, \
									   LT_ERROR, \
									   msg, \
									   operation, \
									   0); \
}

namespace web
{
	enum web_log_e { LT_ERROR, LT_INFO };

	typedef void (*startup_handler_t)(void *server_info, int port);
	typedef void (*shutdown_handler_t)(void *server_info);
	typedef bool (*continue_flag_handler_t)(void *server_info);
	typedef Response (*request_handler_t)(void *server_info,
										  std::string resource_path,
										  std::vector<Header> headers,
										  std::string verb,
										  std::string body);

	typedef void (*log_handler_t)(void *server_info, web_log_e type,
								  const char *s1, const char *s2, int num);

	class ServerInfo
	{
		// (optional) Triggered after the socket is configured.
		startup_handler_t startup_handler;

		// (optional) Triggered after requests are stopped, before the socket is
		// closed.
		shutdown_handler_t shutdown_handler;
		
		// (optional) Receives logging verbosity.
		log_handler_t log_handler;

		// Called regularly. Returns FALSE to immediately quit the main request
		// loop.
		continue_flag_handler_t continue_flag_handler;

		// Request handler.
		request_handler_t request_handler;

		// (optional) Extra request-handler data.
		void *request_handler_data;

		// (optional) Server data. This can be used as a way to keep state between
		// all of the callbacks.
		void *server_data;

		int port;

		public:
			ServerInfo(startup_handler_t startup_handler,
					   shutdown_handler_t shutdown_handler,
					   log_handler_t log_handler,
					   continue_flag_handler_t continue_flag_handler,
					   request_handler_t request_handler,
					   int port=0,
					   void *request_handler_data=NULL,
					   void *server_data=NULL);

			startup_handler_t get_startup_handler() const
				{ return startup_handler; }

			shutdown_handler_t get_shutdown_handler() const
				{ return shutdown_handler; }

			log_handler_t get_log_handler() const { return log_handler; }
			continue_flag_handler_t get_continue_flag_handler() const
				{ return continue_flag_handler; }

			request_handler_t get_request_handler() const
				{ return request_handler; }
			int get_port() const { return port; }
			void *get_request_handler_data() const { return request_handler_data; }
			void *get_server_data() const { return server_data; }

			void set_port(int port_) { port = port_; }
			void set_request_handler_data(void *request_handler_data_)
				{ request_handler_data = request_handler_data_; }

			void set_server_data(void *server_data_)
				{ server_data = server_data_; }
	};

	class RequestContext
	{
		int socketfd;
		ServerInfo *server_info;
		threading::ThreadWrapper *thread;

		public:
			RequestContext(int socketfd, ServerInfo *server_info);

			int GetSocketFd() const { return socketfd; }
			ServerInfo *GetServerInfo() const { return server_info; }
			threading::ThreadWrapper *GetThread() const { return thread; }

			void SetSocketFd(int value) { socketfd = value; }
			void SetServerInfo(ServerInfo *value) { server_info = value; }
			void SetThread(threading::ThreadWrapper *value) { thread = value; }
	};

	int run_server(ServerInfo *server_info, bool start_in_new_thread,
				   threading::ThreadWrapper **main_thread=NULL);

	bool cleanup_server();

	Response get_general_response(int status_code, std::string message,
								  std::string content_type="",
								  std::string reason="",
								  std::string headers="");

	Response get_404_response(std::string resource_path);
}

#endif
