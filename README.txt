Description
===========

A tiny Linux C++ web-server that takes a set of callbacks to manage its 
operation. This projects features a completely rethought body of code that 
began with IBM's open-source small "nweb" C web server 
(http://www.ibm.com/developerworks/systems/library/es-nweb/).

In its simplest form, HWS just extracts the HTTP version, verb, resource path, 
headers, and body from the request, and passes them into a callback. Using the 
"request router" functionality, you can map a number of different prefixes to 
callbacks. Examples of both approaches are in the examples/ directory.

Each incoming request is handled by a new thread. You can choose to start the
main application in a thread of its own (the examples do not). You can also 
tell the platform to bind to a random port (which the examples do).


Why?
====

I was hosting a webpage in an application, and needed a way to communicate from
the Javascript to the backend. I started a webserver instance in another thread
to do this.


Building
========

Eclipse project files are included. Using Eclipse, all but one of the files in
examples/ must be excluded from the build, as they all have versions of main()
in them.

I've also included a makefile. Run "make", and then "bin/<example name>". 
Go to town. 


Developing
==========

When you're ready to implement the functionality, yourself, use the comments
in the "simple" example as a guide.

To start the server, just initialize a ServerInfo object and pass it into
run_server. The following callbacks are required as parameters to the
ServerInfo constructor. You may also pass request-data and server-data into the 
ServerInfo constructor, which will store anonymous data that will be passed 
into the request-handler and into each of the callbacks, respectively.  

void startup_handler(void *server_info, int port)

  Called at server startup.

void shutdown_handler(void *server_info)

  Called at server shutdown.

bool continue_flag_handler(void *server_info)

  Frequently called to determine if requests should still continue to be 
  processed.

Response request_handler(void *server_info, string resource_path,
                         vector<Header> headers, string verb, string body)

  Receives every request. If you're using the router, set the request_handler 
  callback to the router::request_router function, and set request_data (in the
  ServerInfo instance) to an instance of router::RequestRouterData, which, 
  essentially, just holds the mappings.

void log_handler(void *server_info, web_log_e type, const char *s1,
                 const char *s2, int num)

  Receives logging.


License
=======

LGPL
