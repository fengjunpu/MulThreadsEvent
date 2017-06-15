LIBS += ../../lib/event_lib/libevent.a
LIBS += ../../lib/event_lib/libevent_pthreads.a
LIBS += ../../lib/json_lib/libjson.a

main:main.o server.o 
	g++ -o main main.o server.o $(LIBS) -lpthread -lrt
main.o:main.cpp
	g++ -c main.cpp
server.o:server.cpp
	g++ -c server.cpp
clean:
	rm -rf *.o main

