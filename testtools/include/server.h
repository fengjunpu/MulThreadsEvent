#ifndef _H_SERVER_H_
#define _H_SERVER_H_
#include <cassert>
#include <iostream>
#include "../../include/event2/event.h"
#include "../../include/event2/listener.h"
#include "../../include/event2/thread.h"
#include "../../include/event2/bufferevent.h"
#include "../../include/event2/thread.h"
#include "../../include/event2/event_struct.h"

#include "./include/json/json.h"

struct _Clienter{
	struct bufferevent *bufev;
	struct event_base *base;
	std::string index;
	struct event timer;
};


class Server{
public:
	static Server *getInstance();
	bool Server_Start(int,int);
	bool Server_Stop();
	int Thread_Num;
	int Worker_Num;
private:
	Server(){};
	static Server* serobj;
};

typedef struct _Clienter Clienter;
void worker_read_cb(struct bufferevent *bev, void *ctx);
void worker_error_cb(struct bufferevent *bev, short what, void *ctx);
void write_timer_cb(evutil_socket_t fd, short event, void *arg);
void connect_server_cb(evutil_socket_t fd, short event, void *arg);

#endif
