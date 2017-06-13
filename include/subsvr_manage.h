#ifndef __SUBSVR_MANAGE_H__
#define __SUBSVR_MANAGE_H__

#include <string>
#include "redis_define.h"
#include "redis_wrap.h"
#include "match.h"

#ifdef __cplusplus
extern "C" {
#endif
//返回值:  		>=0  成功，< 0  失败
int	start_subsvr_manage(const char *m_server_ip,const char *m_cfgserver_ip);
//返回值:  		>=0  成功，< 0  失败
int	stop_subsvr_manage();
#ifdef __cplusplus
}
#endif
int get_param(char* cfgredisip);

#endif//__SUBSVR_MANAGE_H__

