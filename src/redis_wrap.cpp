/*
 * ds_redis.cpp
 *
 *  Created on: 2015年10月26日
 *      Author: liwangmijia
 */

#include <string.h>
#include <strings.h>

#include "redis_wrap.h"

/*****************************************************************************/

redisContext* redis_connect(const char* host,
        const unsigned int port)
{
    redisContext* connect = redisConnect(host, port);

    if (connect->err) {
        redisFree(connect);
        return NULL;
    }

    return connect;
}

int redis_expire(redisContext* connect,
        const char* key, const unsigned int sec)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "EXPIRE %s %u", key, sec));

    if (NULL != reply &&
            reply->type == REDIS_REPLY_STATUS)
        ret = 0;

    freeReplyObject(reply);

    return ret;
}

int redis_multi(redisContext* connect)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(redisCommand(connect, "MULTI"));

    if (NULL != reply &&
            NULL != reply->str &&
            reply->type == REDIS_REPLY_STATUS &&
            strcasecmp(reply->str, "OK") == 0)
        ret = 0;

    freeReplyObject(reply);

    return ret;
}

int redis_exec(redisContext* connect)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(redisCommand(connect, "EXEC"));

    if (!reply && 0 == reply->elements && reply->type != REDIS_REPLY_ARRAY)
        ret = 0;

    freeReplyObject(reply);

    return ret;
}

int redis_discard(redisContext* connect)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(redisCommand(connect, "DISCARD"));

    if (NULL != reply &&
            NULL != reply->str &&
            reply->type == REDIS_REPLY_STATUS &&
            strcasecmp(reply->str, "OK") == 0)
        ret = 0;

    freeReplyObject(reply);

    return ret;
}

int redis_set(redisContext* connect,
        const char* key, const char* value)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "SET %s %s", key, value));

    if (NULL != reply &&
            NULL != reply->str &&
            reply->type == REDIS_REPLY_STATUS &&
            strcasecmp(reply->str, "OK") == 0)
        ret = 0;

    freeReplyObject(reply);

    return ret;
}

int redis_setex(redisContext* connect,
        const char* key, const char* value)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "SETEX %s %s", key, value));

    if (NULL != reply&&
            NULL != reply->str &&
            reply->type == REDIS_REPLY_STATUS &&
            strcasecmp(reply->str, "OK") == 0)
        ret = 0;

    freeReplyObject(reply);

    return ret;
}

int redis_get(redisContext* connect, const char* key, char* ret_str)
{
    if (!connect)
        return -1;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "GET %s", key));

    if (reply != NULL &&
            reply->str != NULL &&
            reply->type == REDIS_REPLY_STRING)
        memcpy(ret_str, reply->str, reply->len);

    freeReplyObject(reply);

    return 0;
}

int redis_hget(redisContext* connect,
        const char* key, const char* field, char* ret_str)
{
    if (!connect)
        return -1;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "HGET %s %s", key, field));

    if (reply != NULL &&
            reply->str != NULL &&
            reply->type == REDIS_REPLY_STRING)
        memcpy(ret_str, reply->str, reply->len);

    freeReplyObject(reply);

    return 0;
}

int redis_hset(redisContext* connect,
        const char* key,
        const char* field,
        const char* value)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "HSET %s %s %s", key, field, value));

    if (reply != NULL &&
            reply->type == REDIS_REPLY_STATUS)
        ret = 0;

    freeReplyObject(reply);

    return ret;
}

int redis_hdel(redisContext* connect,
        const char* key, const char* field)
{
    int ret = -1;

    if (!connect)
        return ret;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "HDEL %s %s", key, field));

    if (reply != NULL &&
            reply->type == REDIS_REPLY_STATUS)
        ret = 0;

    freeReplyObject(reply);

    return ret;	
}        
std::vector<std::string> redis_hgetall(redisContext* connect,
        const char* key)
{
    std::vector<std::string> retVec;

    if (!connect)
        return retVec;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "HGETALL %s", key));

    if (reply != NULL &&
            reply->elements != 0 &&
            reply->type == REDIS_REPLY_ARRAY) {
        for (std::size_t i = 0; i < reply->elements; ++i) {
            std::string tmpStr;
            tmpStr.assign(reply->element[i]->str, reply->element[i]->len);
            retVec.push_back(tmpStr);
        }
    }

    freeReplyObject(reply);

    return retVec;
}

std::map<std::string,std::string> redis_hgetall_map(redisContext* connect,
        const char* key)
{
    std::map<std::string,std::string> retMap;
	
    if (!connect)
        return retMap;

    redisReply* reply = (redisReply*)(
            redisCommand(connect, "HGETALL %s", key));

    if (reply != NULL &&
            reply->elements != 0 &&
            reply->type == REDIS_REPLY_ARRAY) {
        for (std::size_t i = 0; i < reply->elements-1; i+=2) {
            std::string tmpStrKey;
            std::string tmpStrValue;
            tmpStrKey.assign(reply->element[i]->str, reply->element[i]->len);
            tmpStrValue.assign(reply->element[i+1]->str, reply->element[i+1]->len);            
            retMap[tmpStrKey] = tmpStrValue;
        }
    }

    freeReplyObject(reply);

    return retMap;

}
