CXXFLAGS=-g -Wall

.PHONY: directories

all: directories bin/routed bin/simple

bin/routed: obj/examples/routed.o obj/header.o obj/request_router.o obj/response.o obj/web.o obj/support/thread.o
	g++ $(CXXFLAGS) -o bin/routed obj/examples/routed.o obj/header.o obj/request_router.o obj/response.o obj/web.o obj/support/thread.o -lpthread

bin/simple: obj/examples/simple.o obj/header.o obj/response.o obj/web.o obj/support/thread.o
	g++ $(CXXFLAGS) -o bin/simple obj/examples/simple.o obj/header.o obj/response.o obj/web.o obj/support/thread.o -lpthread

obj/examples/routed.o: examples/routed.cpp
	g++ $(CXXFLAGS) -c -o obj/examples/routed.o examples/routed.cpp

obj/examples/simple.o: examples/simple.cpp
	g++ $(CXXFLAGS) -c -o obj/examples/simple.o examples/simple.cpp

obj/header.o: header.cpp
	g++ $(CXXFLAGS) -c -o obj/header.o header.cpp

obj/request_router.o: request_router.cpp
	g++ $(CXXFLAGS) -c -o obj/request_router.o request_router.cpp

obj/response.o: response.cpp
	g++ $(CXXFLAGS) -c -o obj/response.o response.cpp

obj/web.o: web.cpp
	g++ $(CXXFLAGS) -c -o obj/web.o web.cpp

obj/support/thread.o: support/thread.cpp
	g++ $(CXXFLAGS) -c -o obj/support/thread.o support/thread.cpp
	
directories: bin obj/examples obj/support

bin:
	mkdir -p bin
	
obj/examples:
	mkdir -p obj/examples
	
obj/support:
	mkdir -p obj/support

clean:
	rm -fr bin obj
