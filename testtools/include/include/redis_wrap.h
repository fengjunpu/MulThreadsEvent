/*
 * ds_redis.h
 *
 *  Created on: 2015年10月23日
 *      Author: liwangmijia
 */

#ifndef __REDIS_WRAP_H__
#define __REDIS_WRAP_H__

#include <string>
#include <vector>
#include <map>

//#include "hiredis.h"
#include <hiredis/hiredis.h>
#include <hiredis/async.h>

#ifdef __cplusplus
extern "C" {
#endif

redisContext* redis_connect(const char* host,
        const unsigned int port);

int redis_expire(redisContext* connect,
        const char* key, const unsigned int sec);
int redis_multi(redisContext* connect);
int redis_exec(redisContext* connect);
int redis_discard(redisContext* connect);
int redis_set(redisContext* connect,
        const char* key, const char* value);
int redis_setex(redisContext* connect,
        const char* key, const char* value);
int redis_get(redisContext* connect, const char* key, char* ret_str);
int redis_hget(redisContext* connect,
        const char* key, const char* field, char* ret_str);
int redis_hset(redisContext* connect,
        const char* key,
        const char* field,
        const char* value);
int redis_hdel(redisContext* connect,
        const char* key, const char* field);
std::vector<std::string> redis_hgetall(redisContext* connect,
        const char* key);
std::map<std::string,std::string> redis_hgetall_map(redisContext* connect,
        const char* key);

#ifdef __cplusplus
}
#endif

#endif //__REDIS_WRAP_H__


