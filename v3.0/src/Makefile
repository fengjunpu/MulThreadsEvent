LIBS += ../lib/event_lib/libevent.a
#LIBS += ../lib/event_lib/libevent_pthreads.a
LIBS += ../lib/json_lib/libjson.a
LIBS += ../lib/redis_lib/libhiredis.a
LIBS += ./libmatch.so

#~/Github/dss_access_v2/libevent/lib/libevent_core.a

agent_access:main.o server.o commom_cb.o worker.o subsvr_manage.o common_data.o base_log.o redis_wrap.o
	g++ -std=c++11 -o agent_access main.o server.o worker.o commom_cb.o redis_wrap.o subsvr_manage.o common_data.o base_log.o $(LIBS) $(FLAGS) -lpthread -lrt
main.o:main.cpp
	g++ -std=c++11 -c main.cpp
server.o:server.cpp
	g++ -std=c++11 -c server.cpp
commom_cb.o:commom_cb.cpp
	g++ -std=c++11 -c commom_cb.cpp
worker.o:worker.cpp
	g++ -std=c++11 -c worker.cpp 
redis_wrap.o:redis_wrap.cpp
	g++ -std=c++11 -c redis_wrap.cpp
subsvr_manage.o:subsvr_manage.cpp
	g++ -std=c++11 -c subsvr_manage.cpp 
common_data.o:common_data.cpp
	g++ -std=c++11 -c common_data.cpp
base_log.o:base_log.cpp
	g++ -std=c++11 -c base_log.cpp
clean:
	rm -rf *.o agent_access

