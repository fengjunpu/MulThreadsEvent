#include <sys/socket.h>#include <netinet/in.h>#include <arpa/inet.h>#include <sys/types.h> #include <cstring>#include <pthread.h>#include "../include/server.h"#include "../include/common_cb.h"Server* Server::serobj = NULL;
Server* Server::getInstance(){	if(NULL == serobj)	{		serobj = new Server;
	}
	return serobj;}int Server::Get_Node_Count(){	int count;	pthread_mutex_lock(&s_lock_node_map);	count = dev_node_container.size();	pthread_mutex_unlock(&s_lock_node_map);	return count;}std::string & Server::Get_Redis_Center_IP(){	return redis_center_ip;}void Server::Accept_Conn(struct evconnlistener *ptr, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *arg){	if(NULL == arg)		return ;	Server *server_data = (Server *)arg;	Buffev *worker_buffer = NULL;	int thread_index = 0;	int temp_index = server_data->index;	do{		thread_index = (server_data->index++)%(server_data->m_threadnum);		if(server_data->index == server_data->m_threadnum)			server_data->index = 0;		worker_buffer = server_data->p_worker[thread_index].get_buffev(); 		if(worker_buffer == NULL && (server_data->index == temp_index))		{			std::cout<<"[ERROR]: worker buffer is FULL\n";			EVUTIL_CLOSESOCKET(fd);			return;		}	}while(worker_buffer == NULL);		std::cout<<"thread id = "<<thread_index<<" worker id = "<<worker_buffer->index<<" free number = "<<worker_buffer->owner->freenum<<std::endl;	worker_buffer->timer = event_new(worker_buffer->owner->w_base,-1,EV_TIMEOUT,peer_timeout_cb,worker_buffer);	update_timer_event(worker_buffer->timer,HEART_BEAT_TIMEOUT);		evutil_make_socket_nonblocking(fd);	bufferevent_setfd(worker_buffer->bufev,fd);	bufferevent_enable(worker_buffer->bufev, EV_READ|EV_WRITE);	return;}bool Server::Server_Start(int thread_num,int worker_num){	if(evthread_use_pthreads() != 0) return false;	index = 0;	m_threadnum = thread_num;	pthread_mutex_init(&s_lock_node_map,NULL);	dev_node_container.clear();	base = event_base_new();	assert(base);	struct sockaddr_in addr;	memset(&addr,0,sizeof(addr));	addr.sin_family = AF_INET;	addr.sin_port = htons(6610);	addr.sin_addr.s_addr = inet_addr("0.0.0.0");	evconnlistener_new_bind(base,Accept_Conn,this,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,-1,(struct sockaddr*)&addr,sizeof(addr));	p_worker = new Worker[thread_num];	assert(p_worker);	for(int i = 0;i < thread_num;i++)	{		p_worker[i].w_base = event_base_new();		std::cout<<"index = "<<i<<"  addr = "<<p_worker[i].w_base<<std::endl;		p_worker[i].freenum = worker_num;		Buffev *buf_space = new Buffev[worker_num];		assert(buf_space);		p_worker[i].head = &buf_space[0];		for(int j = 0;j < worker_num;j++)		{			buf_space[j].owner = &p_worker[i];			buf_space[j].bufev = bufferevent_socket_new(p_worker[i].w_base,-1,BEV_OPT_CLOSE_ON_FREE);			assert(buf_space[j].bufev);			buf_space[j].index = j;			buf_space[j].timer = NULL;			buf_space[j].pPeer = NULL;			bufferevent_setcb(buf_space[j].bufev,worker_read_cb, NULL,worker_error_cb,&buf_space[j]);			bufferevent_enable(buf_space[j].bufev, EV_READ|EV_WRITE);			if(j == worker_num -1)			{				buf_space[j].next = NULL;				p_worker[i].tail = &buf_space[j];				break;			}			buf_space[j].next = &buf_space[j+1];	    }				//设置redis相关的事件		p_worker[i].redis_conn_flag = 0;		p_worker[i].r_status  = redisAsyncConnect(REDIS_CENTER_IP,REDIS_STATUS_PORT);		p_worker[i].r_status->data = &p_worker[i];		redisLibeventAttach(p_worker[i].r_status,p_worker[i].w_base);		redisAsyncSetConnectCallback(p_worker[i].r_status,redis_conn_cb);		redisAsyncSetDisconnectCallback(p_worker[i].r_status,redis_disconn_cb);				pthread_t thread;        pthread_create(&thread,NULL,start_base_to_work,p_worker[i].w_base);	}	pthread_t thread;    pthread_create(&thread,NULL,start_base_to_work,base);	return true;}bool Server::Server_Stop(){	std::cout<<"This is func: "<<__func__<<" line: "<<__LINE__<<std::endl;	return true;}