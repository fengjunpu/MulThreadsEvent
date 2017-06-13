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
		std::string uuid = buffev->pPeer->Uuid;
		del_node_from_map(uuid);
		if((buffev->owner->redis_conn_flag == 1) && (uuid.length()))
		{
			redisAsyncCommand(buffev->owner->r_status,redis_op_status,buffev->owner,"DEL %s",uuid.c_str());
		}
		delete buffev->pPeer;
		buffev->pPeer = NULL;
	}
	
	buffev->owner->put_buffev(buffev);
}

void worker_read_cb(struct bufferevent *bev, void *ctx)
{
	if(NULL == ctx)
		return;
	int n,len;
	Buffev *buffev = (Buffev *)ctx;
	char content[MAX_LINE + 1] = {0,};
	while (n = bufferevent_read(bev, content, MAX_LINE),n > 0)
	{
		content[n] = '\0';
	}
	std::cout<<"this is worker read cb count :"<<content<<std::endl;
	parse_http_data(content,buffev);
	return;
}

void worker_error_cb(struct bufferevent *bev, short what, void *ctx)
{
	if(NULL == ctx)
		return;
	Buffev * worker_buffer = (Buffev *)ctx;
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
		delete worker_buffer->pPeer;
		evutil_socket_t fd = bufferevent_getfd(bev);
		evutil_closesocket(fd);
		worker_buffer->pPeer = NULL;
	}
	worker_buffer->owner->put_buffev(worker_buffer);
	std::cout<<"index = "<<worker_buffer->index<<" free number = "<<worker_buffer->owner->freenum<<std::endl;
	return;
}

/*redis 相关*/
void redis_conn_cb(const struct redisAsyncContext* c, int status)
{
	if((NULL == c) || (NULL == c->data))
		return;
	Worker *worker_ev = (Worker *)c->data;
	if(status != REDIS_OK)
	{
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

/*重连redis*/
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

/*redis健康检查*/
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
