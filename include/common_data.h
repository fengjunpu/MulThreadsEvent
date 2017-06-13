#ifndef _H_COMMONDATA_H_
#define _H_COMMONDATA_H_

extern char RPS_SERVER_IP[48];	//rpsaccess 服务器的ip
extern int RPS_SERVER_PORT;		//RPS 服务器的PORT
extern char REDIS_CENTER_IP[48];		//数据中心IP
extern int REDIS_STATUS_PORT;	//状态数据库端口
extern int REDIS_AUTH_PORT;		//授权数据库端口
extern int REDIS_RECONN_INTERNAL;	//Redis重连间隔
extern int HEATER_BEAT_INTERNAL;	//设备心跳间隔时间
extern int HEART_BEAT_TIMEOUT;		//心跳超时时间
extern int REDIS_EXPIRE_TIME;		//数据库中元素的过期时间
extern int REDIS_CHECKHEALTH_INTERNAL; //检查数据库连接状态的时间间隔

#endif
