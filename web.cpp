#include "web.h"

#include <vector>
#include <string>

static pthread_mutex_t locker = PTHREAD_MUTEX_INITIALIZER;
static threading::ThreadWrapper *main_thread;
static std::vector<threading::ThreadWrapper *> finished_threads;

namespace web
{
	Response get_general_response(int status_code, std::string message,
								  std::string content_type, std::string reason,
								  std::string headers)
	{
		if(content_type == "")
			content_type = "text/plain";

		if(reason == "")
			reason = "General";

		std::stringstream ss;
		ss << headers;
		ss << "Content-Type: " << content_type << "\r\n";

		return Response(status_code, reason, message, ss.str().c_str());
	}

	Response get_404_response(std::string resource_path)
	{
		return get_general_response(404,
									resource_path + " not found.",
									"Not found.",
									"Not Found");
	}

	static int web(int fd, ServerInfo *server_info,
				   RequestContext *request_context)
	{
		int buflen;
		long i, ret;

		/* static so zero filled */
		char buffer[BUFSIZE + 1];
	//    char message[200];
		std::stringstream ss;

	// TODO: Read until done.
		/* read Web request in one go */
		ret = read(fd, buffer, BUFSIZE);
		if(ret <= 0)
		{
			/* read failure stop now */
			LOG_RESPOND_ERROR("failed to read browser request.", fd);
			return -1;
		}

		buffer[ret] = 0;

		// Iterate through the headers while we still have room for a two-
		// character newline, and we haven't encountered a double-newline.

		//log_info("Parsing request headers.");

		std::string request_line;
		bool request_line_found = false;
		int found_at;
		std::vector<Header> headers;
		std::string body;

		i = 0;
		while(1)
		{
			char *found_at_raw = strstr(&buffer[i], HNL);
			if(found_at_raw == NULL)
				break;

			found_at = found_at_raw - &buffer[i];

			if(found_at == 0)
				break;

			if(request_line_found == false)
			{
				request_line = std::string(&buffer[i], found_at);
				request_line_found = true;
			}
			else
			{
				std::string text(&buffer[i], found_at);

				unsigned int colon_at;
				if((colon_at = text.find(":")) == std::string::npos)
					break;

				std::string name(text, 0, colon_at);
				std::string value(text, colon_at + 2, (found_at - i) - (colon_at + 2));

				headers.push_back(Header(name, value));
			}

			i += found_at + HNLL;
		}

		if(request_line_found == false)
			return -7;

		body = std::string(&buffer[i + HNLL]);

		i = 0;
		int found_state = 0;
		std::string verb;
		std::string resource_path;
		std::string http_version;
		while(found_state < 3)
		{
			std::string start_at = request_line.substr(i);

			size_t found_at;
			if((found_at = start_at.find(" ")) == std::string::npos)
			{
				if(start_at.substr(0, 5) != "HTTP/")
					return -6;

				http_version = start_at.substr(5);
				break;
			}

			std::string component(request_line, i, found_at);

			if(found_state == 0)
				verb = component;

			else if(found_state == 1)
				resource_path = component;

			i += component.size() + 1;
			found_state++;
		}

		if(verb == "" || resource_path == "" || http_version == "")
		{
			LOG_RESPOND_ERROR("Request-line was not completely parsed.", fd);
			return -4;
		}

		request_handler_t handler = server_info->get_request_handler();
		Response response = handler(server_info, resource_path, headers, verb,
									 body);

		ss.clear();
		ss.str(std::string());

		ss << "HTTP/"
		   << std::showpoint << std::setprecision(2)
		   << response.get_http_version() << " "
		   << response.get_status_code() << " "
		   << response.get_reason_phrase() << "\r\n"
		   << response.get_response_headers() << "\r\n"
		   << response.get_response_body();

		std::string response_text = ss.str();

		i = 0;
		int sent;
		buflen = response_text.size();
		int send_length;
		while(i < buflen)
		{
			send_length = std::min(BUFSIZE, buflen - i);
			sent = write(fd, &response_text[i], send_length);

			if(sent < BUFSIZE)
				break;

			i += BUFSIZE;
		}

		return 0;
	}

	bool handle_request(void *requestContextRaw)
	{
		RequestContext *request_context = (RequestContext *)requestContextRaw;

		int socketfd = request_context->GetSocketFd();
		ServerInfo *server_info = request_context->GetServerInfo();

		int result = web(socketfd, server_info, request_context);

		// This was opened when we received this particular request, before the
		// thread spawned.
		close(socketfd);

		pthread_mutex_lock(&locker);
		finished_threads.push_back(request_context->GetThread());
		pthread_mutex_unlock(&locker);

		// This was allocated just before our thread was spawned. Free our
		// arbitrary thread data.
		delete request_context;

		if(result != 0)
		{
			LOG_ERROR_SOCKET("Server instance returned failure.", "web");
			return false;
		}

		return true;
	}

	static bool cleanup_finished_threads()
	{
		pthread_mutex_lock(&locker);

		while(finished_threads.empty() == false)
		{
			threading::ThreadWrapper *thread = finished_threads[0];
			finished_threads.erase(finished_threads.begin());

			//stringstream ss;
			//ss << "Cleaning-up thread [" << thread << "].";
			//log_info(ss.str());

			thread->Join();
			delete thread;
		}

		pthread_mutex_unlock(&locker);

		return true;
	}
	
	bool run_server_internal(ServerInfo *server_info)
	{
		int listenfd;
		int socketfd;
		socklen_t length;
		static struct sockaddr_in cli_addr; /* static = initialised to zeros */
		static struct sockaddr_in serv_addr; /* static = initialised to zeros */

		int port = server_info->get_port();

		if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			LOG_ERROR_SOCKET("system call", "socket");
			return false;
		}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(port);

		if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			LOG_ERROR_SOCKET("system call", "bind");
			return false;
		}

		if(listen(listenfd, 64) <0)
		{
			LOG_ERROR_SOCKET("system call", "listen");
			return false;
		}

		if(port == 0)
		{
			unsigned int addrlen = sizeof(serv_addr);
			if(getsockname(listenfd, (struct sockaddr *)&serv_addr, &addrlen) != 0)
			{
				LOG_ERROR_SOCKET("system call", "getsockname");
				return false;
			}

			port = ntohs(serv_addr.sin_port);
		}

		server_info->set_port(port);

		if(server_info->get_startup_handler())
			server_info->get_startup_handler()(server_info, port);

		pthread_mutex_init(&locker, NULL);

		struct pollfd poll_sockets[] = {
			{ listenfd, POLLIN }
		};

		int error = 0;

		while(1)
		{
			cleanup_finished_threads();

			bool quit = false;
			while(error == 0)
			{
				if(server_info->get_continue_flag_handler()(server_info) == false)
				{
					quit = true;
					break;
				}

				// Use a select to detect a connection without having to sit on an
				// accept() indefinitely. This allows us to very frequently check
				// server_info.precycle_handler(), which will allow us to quit
				// whenever the host application wants us to (like on a
				// CTRL+BREAK).

				int ready = poll(poll_sockets, 3, 1000);
				if(ready < 0)
				{
					LOG_ERROR_SOCKET("system call", "poll");

					error = 4;
					break;
				}
				else if(ready == 0)
					continue;

				length = sizeof(cli_addr);
				if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr,
									  &length)) < 0)
				{
					LOG_ERROR_SOCKET("system call", "accept");

					error = 2;
					break;
				}

				break;
			}

			if(quit || error != 0)
				break;

			RequestContext *request_context = new RequestContext(socketfd,
																 server_info);

			threading::ThreadWrapper *thread;
			thread = new threading::ThreadWrapper("Request worker.",
												  handle_request,
												  request_context);

			request_context->SetThread(thread);

			if(thread->Start() == false)
			{
				error = 3;
				break;
			}
		}

		cleanup_finished_threads();

		pthread_mutex_destroy(&locker);

		if(server_info->get_shutdown_handler())
			server_info->get_shutdown_handler()(server_info);

		close(listenfd);

		return (error == 0);
	}

	bool run_server_in_thread(void *server_info_raw)
	{
		ServerInfo *server_info = (ServerInfo *)server_info_raw;

		// If this fails, the main-server thread will timeout waiting for a port,
		// and terminate.
		run_server_internal(server_info);

		delete main_thread;

		return true;
	}

	int run_server(ServerInfo *server_info, bool start_in_new_thread,
				   threading::ThreadWrapper **main_thread_)
	{
		if(start_in_new_thread == false)
		{
			return run_server_internal(server_info);
	// TODO: Clean-up server-info allocations (when NOT running as a thread).
		}
		else
		{
			main_thread = new threading::ThreadWrapper("Web server.",
													   run_server_in_thread,
											           server_info);

			if(main_thread_ != NULL)
				*main_thread_ = main_thread;

			if(main_thread->Start() == false)
				return -99;
		}
	
		return 0;
	}

	ServerInfo::ServerInfo(startup_handler_t startup_handler_,
						   shutdown_handler_t shutdown_handler_,
						   log_handler_t log_handler_,
						   continue_flag_handler_t continue_flag_handler_,
						   request_handler_t request_handler_,
						   int port_,
						   void *request_handler_data_,
						   void *server_data_)
	{
		startup_handler = startup_handler_;
		shutdown_handler = shutdown_handler_;
		log_handler = log_handler_;
		continue_flag_handler = continue_flag_handler_;
		request_handler = request_handler_;
		port = port_;
		request_handler_data = request_handler_data_;
		server_data = server_data_;
	}

	RequestContext::RequestContext(int socketfd_, ServerInfo *server_info_)
	{
		socketfd = socketfd_;
		server_info = server_info_;
		thread = NULL;
	}
}
