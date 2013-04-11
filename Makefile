all: bin/web

obj/cresponse.obj: cresponse.cpp cresponse.h
	g++ -c -o obj/cresponse.obj cresponse.cpp

obj/cheader.obj: cheader.cpp cheader.h
	g++ -c -o obj/cheader.obj cheader.cpp

obj/string_util.obj: string_util.cpp string_util.h
	g++ -c -o obj/string_util.obj string_util.cpp

bin/web: web.cpp web.h main.cpp obj/cresponse.obj obj/cheader.obj obj/string_util.obj
	g++ -g -o bin/web web.cpp main.cpp obj/cresponse.obj obj/cheader.obj obj/string_util.obj

clean:
	rm obj/* bin/*

