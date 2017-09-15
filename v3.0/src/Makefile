LIBS += ../lib/event_lib/libevent.a
LIBS += ../lib/event_lib/libevent_pthreads.a
LIBS += ../lib/json_lib/libjson.a
LIBS += ../lib/redis_lib/libhiredis.a
LIBS += ./libmatch.so


agent_access:main.o server.o commom_cb.o worker.o subsvr_manage.o common_data.o
	g++ -std=c++11 -o agent_access main.o server.o worker.o commom_cb.o subsvr_manage.o common_data.o $(LIBS) -lpthread -lrt
main.o:main.cpp
	g++ -std=c++11 -c main.cpp
server.o:server.cpp
	g++ -std=c++11 -c server.cpp
commom_cb.o:commom_cb.cpp
	g++ -std=c++11 -c commom_cb.cpp
worker.o:worker.cpp
	g++ -std=c++11 -c worker.cpp
subsvr_manage.o:subsvr_manage.cpp
	g++ -std=c++11 -c subsvr_manage.cpp
common_data.o:common_data.cpp
	g++ -std=c++11 -c common_data.cpp
clean:
	rm -rf *.o agent_access

