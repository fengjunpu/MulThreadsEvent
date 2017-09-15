#ifndef _H_COMMONCB_H_
#define _H_COMMONCB_H_
#include "worker.h"
#include "server.h"

void *start_base_to_work(void *arg);   //驱动base的线程
void worker_read_cb(struct bufferevent *bev, void *ctx);
void worker_error_cb(struct bufferevent *bev, short what, void *ctx);
void peer_timeout_cb(evutil_socket_t fd, short ev, void *ctx);
void redis_conn_cb(const struct redisAsyncContext*, int status);
void redis_disconn_cb(const struct redisAsyncContext*, int status);
void redis_reconn_cb(evutil_socket_t fd, short event, void *ctx);
void redis_check_health_cb(evutil_socket_t fd, short event, void *ctx);
void redis_op_status(redisAsyncContext *c, void * redis_reply, void * arg);
#endif