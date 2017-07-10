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
	Buffev *buffev = (Buffev *)ctx;
	if(buffev->timer != NULL)
	{
		event_free(buffev->timer);
		buffev->timer = NULL;
	}
	
	bufferevent_disable(buffev->bufev,EV_READ | EV_WRITE);
	evutil_socket_t buffd = bufferevent_getfd(buffev->bufev);
	evutil_closesocket(buffd);
	if(buffev->pPeer != NULL)
	{
		std::cout<<"ERROR:Peer TimeOut index = "<<buffev->pPeer->Uuid<<std::endl;
		std::string uuid = buffev->pPeer->Uuid;
		del_node_from_map(uuid);
		if((buffev->owner->redis_conn_flag == 1) && (uuid.length()))
		{
			redisAsyncCommand(buffev->owner->r_status,redis_op_status,buffev->owner,"DEL %s",uuid.c_str());
		}
		delete buffev->pPeer;
		buffev->pPeer = NULL;
	}
	std::cout<<"ERROR:Buffev TimeOut owner index = "<<buffev->index<<std::endl;
	buffev->owner->put_buffev(buffev);
}

void worker_read_cb(struct bufferevent *bev, void *ctx)
{
	if(NULL == ctx)
		return;
	Buffev *buffev = (Buffev *)ctx;
	int n = 0,len = 0;

	char content[MAX_LINE + 1] = {0,};
	while (n = bufferevent_read(bev, content+len, MAX_LINE),n > 0)
	{
		len += n;
		content[len] = '\0';
	}
	parse_http_data(content,buffev);
	/*
	struct evbuffer *pinbuf = bufferevent_get_input(bev);
	assert(pinbuf != NULL);
	int len = evbuffer_get_length(pinbuf);
	assert(len >= 0);
	if(len == 0)
		return;
	char *pmsg = (char *)evbuffer_pullup(pinbuf, -1);
	std::string content(pmsg,len);
	parse_http_data((char *)content.c_str(),buffev);
	*/
	return;
}

void worker_error_cb(struct bufferevent *bev, short what, void *ctx)
{
	if(NULL == ctx)
		return;
	Buffev * worker_buffer = (Buffev *)ctx;
	evutil_socket_t fd = bufferevent_getfd(bev);
	evutil_closesocket(fd);
	bufferevent_disable(bev,EV_READ | EV_WRITE);
	if(worker_buffer->timer != NULL)
	{
		event_free(worker_buffer->timer);
		worker_buffer->timer = NULL;
	}
	
	if(worker_buffer->pPeer != NULL)
	{
		del_node_from_map(worker_buffer->pPeer->Uuid);
		std::string uuid = worker_buffer->pPeer->Uuid;
		if((worker_buffer->owner->redis_conn_flag == 1) && (uuid.length()))
		{
			redisAsyncCommand(worker_buffer->owner->r_status,redis_op_status,worker_buffer->owner,"DEL %s",uuid.c_str());
		}
		std::cout<<"free pPeer\n";
		delete worker_buffer->pPeer;
		worker_buffer->pPeer = NULL;
	}
//	bufferevent_free(bev);
	worker_buffer->owner->put_buffev(worker_buffer);
	std::cout<<"index = "<<worker_buffer->index<<" free number = "<<worker_buffer->owner->freenum<<std::endl;
	std::cout<<"1111111111111111111\n";
	return;
}

/*redis ���*/
void redis_conn_cb(const struct redisAsyncContext* c, int status)
{
	if((NULL == c) || (NULL == c->data))
		return;
	Worker *worker_ev = (Worker *)c->data;
	if(status != REDIS_OK)
	{
		std::cout<<"connect Redis failed\n";
		worker_ev->redis_conn_flag = 0;
		if(event_initialized(&worker_ev->ev))
			event_del(&worker_ev->ev);
		event_assign(&worker_ev->ev,worker_ev->w_base,-1,0,redis_reconn_cb,worker_ev);
		update_timer_event(&worker_ev->ev,REDIS_RECONN_INTERNAL);
	}
	else
	{
		worker_ev->redis_conn_flag = 1;
		if(event_initialized(&worker_ev->ev))
			event_del(&worker_ev->ev);
		event_assign(&worker_ev->ev,worker_ev->w_base,-1,0,redis_check_health_cb,worker_ev);
		update_timer_event(&worker_ev->ev,REDIS_CHECKHEALTH_INTERNAL);
	}
}

void redis_disconn_cb(const struct redisAsyncContext* c, int status)
{
	if((NULL == c) || (NULL == c->data))
		return;
	Worker *worker_ev = (Worker *)c->data;
	worker_ev->redis_conn_flag = 0;
	if(event_initialized(&worker_ev->ev))
		event_del(&worker_ev->ev);
	event_assign(&worker_ev->ev,worker_ev->w_base,-1,0,redis_reconn_cb,worker_ev);
	update_timer_event(&worker_ev->ev,REDIS_RECONN_INTERNAL);
}

/*����redis*/
void redis_reconn_cb(evutil_socket_t fd, short event, void *ctx)
{
	if(NULL == ctx)
		return;
	Worker *worker_ev = (Worker *)ctx;
	worker_ev->r_status  = redisAsyncConnect(REDIS_CENTER_IP,REDIS_STATUS_PORT);
	worker_ev->r_status->data = worker_ev;
	redisLibeventAttach(worker_ev->r_status,worker_ev->w_base);
	redisAsyncSetConnectCallback(worker_ev->r_status,redis_conn_cb);
	redisAsyncSetDisconnectCallback(worker_ev->r_status,redis_disconn_cb);
	return;
}

/*redis�������*/
void redis_check_health_cb(evutil_socket_t fd, short event, void *ctx)
{
	if(NULL == ctx)
		return;
	Worker * worker_ev = (Worker *)ctx;
	if(worker_ev->redis_conn_flag == 1)
	{
		redisAsyncCommand(worker_ev->r_status, redis_op_status,worker_ev, "SET Test Test");
	}
	return ;
}

void redis_op_status(redisAsyncContext *c, void * redis_reply, void * arg)
{
	if(arg == NULL)
		return;
	Worker * worker_ev = (Worker *)arg;	
	redisReply * reply   = (redisReply *)redis_reply;
	if(NULL == reply)
	{
		redisAsyncDisconnect(c);
		return;
	}
	update_timer_event(&worker_ev->ev,REDIS_CHECKHEALTH_INTERNAL);
}
