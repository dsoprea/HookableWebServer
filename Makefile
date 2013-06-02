all: bin/routed

bin/routed: obj/examples/routed.o obj/header.o obj/request_router.o obj/response.o obj/web.o obj/support/thread.o
	mkdir -p bin
	g++ -o bin/routed obj/examples/routed.o obj/header.o obj/request_router.o obj/response.o obj/web.o obj/support/thread.o -lpthread

obj/examples/routed.o: examples/routed.cpp
	mkdir -p obj/examples
	g++ -c -o obj/examples/routed.o examples/routed.cpp

obj/header.o: header.cpp
	mkdir -p obj
	g++ -c -o obj/header.o header.cpp

obj/request_router.o: request_router.cpp
	mkdir -p obj
	g++ -c -o obj/request_router.o request_router.cpp

obj/response.o: response.cpp
	mkdir -p obj
	g++ -c -o obj/response.o response.cpp

obj/web.o: web.cpp
	mkdir -p obj
	g++ -c -o obj/web.o web.cpp

obj/support/thread.o: support/thread.cpp
	mkdir -p obj/support
	g++ -c -o obj/support/thread.o support/thread.cpp

clean:
	rm -fr bin obj
	
	