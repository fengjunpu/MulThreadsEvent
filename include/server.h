#ifndef _H_SERVER_H_
#define _H_SERVER_H_
#include <cassert>
#include <iostream>
#include "./event2/event.h"
#include "./event2/listener.h"
#include "./event2/thread.h"
#include "./event2/bufferevent.h"
#include "./event2/thread.h"
#include "worker.h"

class Server{
public:
	static Server *getInstance();
	static void Accept_Conn(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
	bool Server_Start(int,int);
	bool Server_Stop();
	int m_threadnum;
	int index;
	pthread_mutex_t s_lock_node_map;
	Worker *p_worker;
	Node_Map dev_node_container;
private:
	Server(){};
	static Server* serobj;
	struct event_base *base;
};

#endif
