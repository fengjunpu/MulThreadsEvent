#ifndef _H_WORKER_H_
#define _H_WORKER_H_
#include <string>
#include <map>
#include <vector>
#include <cstdlib>

#include "json/json.h"
#include "./event2/event_struct.h"
#include "./hredis/hiredis.h"
#include "./hredis/async.h"
#include "./hredis/adapters/libevent.h"

struct _Conn;
struct _Node;
struct _SyRedis;

typedef std::map<std::string, std::string> map_str2str_t;

struct _Conn{
	struct bufferevent *bufev;
	struct event *timer;
	struct _Node *pPeer;
	struct event_base *base;
	struct _SyRedis *hredis;
};

struct _Node{
	_Conn *pConn;
	int DevPort;
	int FlushRedis;
	char Area[64];
	char Oemid[64];
	char Uuid[32];
};

struct _SyRedis{
	int redis_conn_flag; 	
	redisAsyncContext *r_status;
	struct event_base *w_base;
	struct event ev;
};

typedef struct
{
    std::string type;
    std::string method;
    std::string url;
    int status_code;
    map_str2str_t headers;
    char *body;
    int bodylen;
    int msglen;
}http_msg_t;

typedef struct _Conn Conn;
typedef struct _Node Node;
typedef struct _SyRedis SyRedis;

typedef std::map<std::string,Node *> Node_Map;
typedef std::vector<std::string> vect_str_t;

std::vector<std::string> split(const std::string &s, char delim);

int handle_client_rquest(http_msg_t *data,Conn *buff);
int parse_http_msg(char *pbuf, int len, http_msg_t *phttp_msg);
int handle_dev_register(Conn *buffev,std::string uuid,std::string are,std::string oenmid,std::string port);
int handle_conn_requst(Conn *buffev,std::string uuid,std::string token,std::string servertype,std::string sesionid,std::string destport);
bool del_node_from_map(std::string uuid);
bool insert_node_to_map(std::string uuid,Node *peer);
Node *get_node_from_map(std::string &uuid);
bool update_timer_event(struct event *ev,int time);
int error_rps_data(struct bufferevent *bufev,int code);

#endif
