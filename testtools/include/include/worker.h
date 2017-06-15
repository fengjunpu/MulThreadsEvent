#ifndef _H_WORKER_H_
#define _H_WORKER_H_
#include <string>
#include <map>
#include <cstdlib>

#include "json/json.h"
#include "./event2/event_struct.h"
#include "./hredis/hiredis.h"
#include "./hredis/async.h"
#include "./hredis/adapters/libevent.h"

struct _Buffev;
struct _Worker;
struct _Node;

struct _Buffev{
	struct bufferevent *bufev;
	struct event *timer;
	int index;
	_Node *pPeer;
	_Worker *owner;
	_Buffev *next;
};

struct _Worker{
	struct event_base *w_base;
	struct event ev;
	redisAsyncContext *r_status;
	int redis_conn_flag;  //0 未连接 1已连接
	_Buffev* head;
	_Buffev* tail;
	int freenum;
	inline _Buffev *get_buffev()
	{
		if(NULL == head)
			return NULL;
		_Buffev *ret = head;
		head = head->next;
		ret->next = NULL;
		freenum--;
		return ret;
	}
	
	inline bool put_buffev(_Buffev *bufev)
	{
		if(NULL == head)
		{
			head=tail=bufev;
		}
		else
		{
			tail->next = bufev;
			tail = bufev;
		}
		bufev->next = NULL;
		freenum++;
		return true;
	}
};

struct _Node{
	_Buffev *WorBuf;
	int DevPort;
	int FlushRedis;
	std::string Area;
	std::string Oemid;
	std::string Uuid;
};

typedef struct _Buffev Buffev;
typedef struct _Worker Worker;
typedef struct _Node Node;
typedef std::map<std::string,Node *> Node_Map;

int parse_http_data(char *data,Buffev *buff);
int handle_dev_register(Buffev *buffev,std::string uuid,std::string are,std::string oenmid,std::string port);
int handle_conn_requst(Buffev *buffev,std::string uuid,std::string token,std::string servertype,std::string sesionid,std::string destport);
bool del_node_from_map(std::string uuid);
bool insert_node_to_map(std::string uuid,Node *peer);
Node *get_node_from_map(std::string &uuid);
bool update_timer_event(struct event *ev,int time);

#endif
