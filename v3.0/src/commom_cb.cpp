#include <iostream>
#include "../include/common_cb.h"
#include "../include/worker.h"

#define MAX_LINE 2048

void * start_base_to_work(void *arg)
{
	if(NULL == arg)
		return NULL;
	struct event_base *base = (struct event_base *)arg;
	event_base_dispatch(base);
	return base;
}

void peer_timeout_cb(evutil_socket_t fd, short ev, void *ctx)
{
	if(NULL == ctx)
		return;
	Conn *m_conn = (Conn *)ctx;
	if(m_conn->timer != NULL)
	{
		if(event_initialized(m_conn->timer)) 
		{
			event_del(m_conn->timer);
		}
		
		event_free(m_conn->timer);
		m_conn->timer = NULL;
	}
	
	if(m_conn->bufev != NULL)
	{
		bufferevent_setcb(m_conn->bufev, NULL, NULL, NULL, NULL);
		bufferevent_free(m_conn->bufev);
	}
	
	if(m_conn->pPeer != NULL)
	{
		LOG(ERROR)<<"ERROR:Peer TimeOut index = "<<m_conn->pPeer->Uuid;
		std::string uuid = m_conn->pPeer->Uuid;
		del_node_from_map(uuid);
		if((m_conn->hredis->redis_conn_flag == 1) && (uuid.length()))
			redisAsyncCommand(m_conn->hredis->r_status,redis_op_status,m_conn->hredis,"DEL %s",uuid.c_str());
		free(m_conn->pPeer);
		m_conn->pPeer = NULL;
	}
	
	free(m_conn);
	m_conn = NULL;
}

void worker_read_cb(struct bufferevent *bev, void *ctx)
{
	if(NULL == ctx)
		return;
	http_msg_t http_msg;
	Conn *conn = (Conn *)ctx;
	struct evbuffer *pinbuf = bufferevent_get_input(bev);
	assert(pinbuf != NULL);
	
	char *pmsg = (char *)evbuffer_pullup(pinbuf, -1);
    assert(pmsg != NULL);
	
	int len = evbuffer_get_contiguous_space(pinbuf);
	if(len == 0)
		return ;
	
	std::string strmsg(pmsg, len);	
	int ret = parse_http_msg((char *)strmsg.c_str(),len,&http_msg);
	if(ret <= 0)
	{
		if(ret < 0) 
		{
			error_rps_data(bev,HTTP_RES_400);
		}
		return ;
	}
	
	handle_client_rquest(&http_msg,conn);
	evbuffer_drain(pinbuf,http_msg.msglen);
	
	return;
}

void worker_error_cb(struct bufferevent *bev, short what, void *ctx)
{
	if(NULL == ctx)
		return;
	
	Conn *worker_conn = (Conn *)ctx;
	if(worker_conn->timer != NULL)
	{
		if(event_initialized(worker_conn->timer)) 
		{
			event_del(worker_conn->timer);
		}
		
		event_free(worker_conn->timer);
		worker_conn->timer = NULL;
	}
	
	if(worker_conn->bufev != NULL)
	{
		bufferevent_setcb(worker_conn->bufev, NULL, NULL, NULL, NULL);
		bufferevent_free(worker_conn->bufev);
		worker_conn->bufev = NULL;
	}
	
	if(worker_conn->pPeer != NULL)
	{
		LOG(ERROR)<<"Error: connection is failed uuid: "<<worker_conn->pPeer->Uuid;
		del_node_from_map(worker_conn->pPeer->Uuid);
		std::string uuid = worker_conn->pPeer->Uuid;
		if((worker_conn->hredis->redis_conn_flag == 1) && (uuid.length()))
			redisAsyncCommand(worker_conn->hredis->r_status,redis_op_status,worker_conn->hredis,"DEL %s",uuid.c_str());
		free(worker_conn->pPeer);
		worker_conn->pPeer = NULL;
	}
	
	free(worker_conn);
	worker_conn = NULL;
	return;
}

/*redis 相关*/
void redis_conn_cb(const struct redisAsyncContext* c, int status)
{
	if((NULL == c) || (NULL == c->data))
		return;
	SyRedis *redis_ev = (SyRedis *)c->data;
	if(status != REDIS_OK)
	{
		redis_ev->redis_conn_flag = 0;
		if(event_initialized(&redis_ev->ev))
			event_del(&redis_ev->ev);
		event_assign(&redis_ev->ev,redis_ev->w_base,-1,0,redis_reconn_cb,redis_ev);
		update_timer_event(&redis_ev->ev,REDIS_RECONN_INTERNAL);
	}
	else
	{
		redis_ev->redis_conn_flag = 1;
		if(event_initialized(&redis_ev->ev))
			event_del(&redis_ev->ev);
		event_assign(&redis_ev->ev,redis_ev->w_base,-1,0,redis_check_health_cb,redis_ev);
		update_timer_event(&redis_ev->ev,REDIS_CHECKHEALTH_INTERNAL);
	}
}

void redis_disconn_cb(const struct redisAsyncContext* c, int status)
{
	if((NULL == c) || (NULL == c->data))
		return;
	SyRedis *redis_ev = (SyRedis *)c->data;
	redis_ev->redis_conn_flag = 0;
	if(event_initialized(&redis_ev->ev))
		event_del(&redis_ev->ev);
	event_assign(&redis_ev->ev,redis_ev->w_base,-1,0,redis_reconn_cb,redis_ev);
	update_timer_event(&redis_ev->ev,REDIS_RECONN_INTERNAL);
}

/*重连redis*/
void redis_reconn_cb(evutil_socket_t fd, short event, void *ctx)
{
	if(NULL == ctx)
		return;
	SyRedis *redis_ev = (SyRedis *)ctx;
	redis_ev->r_status  = redisAsyncConnect(REDIS_CENTER_IP,REDIS_STATUS_PORT);
	redis_ev->r_status->data = redis_ev;
	redisLibeventAttach(redis_ev->r_status,redis_ev->w_base);
	redisAsyncSetConnectCallback(redis_ev->r_status,redis_conn_cb);
	redisAsyncSetDisconnectCallback(redis_ev->r_status,redis_disconn_cb);
	return;
}

/*redis健康检查*/
void redis_check_health_cb(evutil_socket_t fd, short event, void *ctx)
{
	if(NULL == ctx)
		return;
	SyRedis * redis_ev = (SyRedis *)ctx;
	if(redis_ev->redis_conn_flag == 1)
	{
		redisAsyncCommand(redis_ev->r_status,redis_op_status,redis_ev, "SET Test Test");
	}
	return ;
}

void redis_op_status(redisAsyncContext *c, void * redis_reply, void * arg)
{
	if(arg == NULL)
		return;
	SyRedis * redis_ev = (SyRedis *)arg;	
	redisReply * reply   = (redisReply *)redis_reply;
	if(NULL == reply)
	{
		LOG(ERROR)<<"redis health check connection disconected";
		redisAsyncDisconnect(c);
		return;
	}
	update_timer_event(&redis_ev->ev,REDIS_CHECKHEALTH_INTERNAL);
}
