#include <cstring>
#include <cstdlib>
#include <sstream>
#include <string>

#include "../include/worker.h"
#include "../include/server.h"
#include "../include/common_cb.h"
#include "../include/match.h"

#define DEV_REGIREQ "MSG_AGENT_REGISTER_REQ"
#define DEV_REGIRSP "MSG_AGENT_REGISTER_RSP"
#define CLI_CONNQEQ "MSG_CLI_NEED_CON_REQ"
#define CLI_CONNRSP "MSG_CLI_NEED_CON_RSP"
#define DEV_CONNREQ "MSG_DEV_START_CON"

static std::string getString(const Json::Value & table)
{
    std::string temp;
    Json::StyledWriter writer(temp);
    writer.write(table);
    return temp;
}

bool update_timer_event(struct event *ev,int time)
{
	if(NULL == ev)
		return false;
	struct timeval tv;	
	evutil_timerclear(&tv);	
	tv.tv_sec = time;
	tv.tv_usec = 0;
	event_add(ev,&tv);
	return true;
}

Node *get_node_from_map(std::string &uuid)
{
	pthread_mutex_lock(&Server::getInstance()->s_lock_node_map);
	Node *pPeer = NULL;
	Node_Map::iterator it = Server::getInstance()->dev_node_container.find(uuid);
	if(it != Server::getInstance()->dev_node_container.end())
	{
		pPeer = (*it).second;
	}
	pthread_mutex_unlock(&Server::getInstance()->s_lock_node_map);
	return pPeer;
}

bool insert_node_to_map(std::string uuid,Node *peer)
{
	if(NULL == peer)
		return false;
	pthread_mutex_lock(&Server::getInstance()->s_lock_node_map);
	Server::getInstance()->dev_node_container.insert(Node_Map::value_type(uuid, peer));
	pthread_mutex_unlock(&Server::getInstance()->s_lock_node_map);
	return true;
}

bool del_node_from_map(std::string uuid)
{
	pthread_mutex_lock(&Server::getInstance()->s_lock_node_map);
	Server::getInstance()->dev_node_container.erase(uuid);
	pthread_mutex_unlock(&Server::getInstance()->s_lock_node_map);
	return true;
}


/*解析http数据数据*/
int parse_http_data(char *data,Buffev *buffev)
{
	if(NULL == data)
		return -1;
	char *body = strstr(data,"\r\n\r\n");
	if(NULL == body)
		return -1;
	Json::Reader    reader;
    Json::Value     requestValue;
	if(reader.parse(body, requestValue) == false)
   	{
		return -1;
   	}
	
	if((requestValue.isObject())&&(requestValue.isMember("AgentProtocol"))&& \
		(requestValue["AgentProtocol"].isMember("Header"))&& \
		(requestValue["AgentProtocol"]["Header"].isMember("MessageType")))
	{
		std::string MessageType = requestValue["AgentProtocol"]["Header"]["MessageType"].asCString();
		std::string Uuid = requestValue["AgentProtocol"]["Body"]["SerialNumber"].asCString();
		if(DEV_REGIREQ == MessageType)
		{
			bool Ret = false;
			Ret	= requestValue["AgentProtocol"]["Body"].isMember("Area");
			std::string Area = Ret ? requestValue["AgentProtocol"]["Body"]["Area"].asCString() : "Default:Default:Default";

			Ret = requestValue["AgentProtocol"]["Body"].isMember("RewriteOemID");
			std::string OemId = Ret ? requestValue["AgentProtocol"]["Body"]["RewriteOemID"].asCString() : "General";

			Ret = requestValue["AgentProtocol"]["Body"].isMember("DevicePort");
			std::string StrPort = Ret ? requestValue["AgentProtocol"]["Body"]["RewriteOemID"].asCString() : "-1";
			
			return handle_dev_register(buffev,Uuid,Area,OemId,StrPort);
		}
		else if(CLI_CONNQEQ == MessageType)
		{
			bool Ret = false;
			do
			{
				Ret = requestValue["AgentProtocol"]["Body"].isMember("ClientToken");
				if(!Ret) break;
				Ret = requestValue["AgentProtocol"]["Body"].isMember("ServiceType");
				if(!Ret) break;
				Ret = requestValue["AgentProtocol"]["Body"].isMember("SessionId");
				if(!Ret) break;
				Ret = requestValue["AgentProtocol"]["Body"].isMember("DestPort");
				if(!Ret) break;
				std::string ClientToken = requestValue["AgentProtocol"]["Body"]["ClientToken"].asCString();
				std::string ServerType = requestValue["AgentProtocol"]["Body"]["ServiceType"].asString();
				std::string SessionId = requestValue["AgentProtocol"]["Body"]["SessionId"].asCString();
				std::string DestPort = requestValue["AgentProtocol"]["Body"]["DestPort"].asCString();
				return handle_conn_requst(buffev,Uuid,ClientToken,ServerType,SessionId,DestPort);
			}while(0);
		}
	}
	/*Bad Request ... ...*/
}


/*处理设备注册函数*/
int handle_dev_register(Buffev *buffev,std::string uuid,std::string are,std::string oenmid,std::string port)
{
	Node * pPeer = get_node_from_map(uuid);
	if((NULL == pPeer) || (pPeer->WorBuf != buffev))
	{
		int flag = 0;
		if(NULL == pPeer)
		{
			pPeer = new Node;
			assert(pPeer);
			flag = 1;
			
		}
		else
		{
			if(pPeer->WorBuf->timer != NULL)
			{
				event_free(pPeer->WorBuf->timer);
				pPeer->WorBuf->timer = NULL;
			}
			bufferevent_disable(pPeer->WorBuf->bufev,EV_READ | EV_WRITE);
			evutil_socket_t fd = bufferevent_getfd(pPeer->WorBuf->bufev);
			evutil_closesocket(fd);
			pPeer->WorBuf->pPeer = NULL;
			pPeer->WorBuf->owner->put_buffev(pPeer->WorBuf);
		}
		pPeer->FlushRedis = 0;
		pPeer->WorBuf = buffev;
		pPeer->DevPort = atoi(port.c_str());
		pPeer->Area = are;
		pPeer->Oemid = oenmid;	
		pPeer->Uuid = uuid;
		pPeer->WorBuf->pPeer = pPeer;
		if(1 == flag)
			insert_node_to_map(uuid,pPeer);
		
	}
	update_timer_event(buffev->timer,HEART_BEAT_TIMEOUT);
	
	/*更新数据库*/
	struct timeval nowtv;
	evutil_gettimeofday(&nowtv, NULL);
	if(buffev->owner->redis_conn_flag == 1 &&(nowtv.tv_sec - pPeer->FlushRedis > 150))
	{
		redisAsyncCommand(buffev->owner->r_status, redis_op_status,buffev->owner, "HMSET %s ServerIP %s ServerPort %d DevicePort %d", \
                                        uuid.c_str(),REDIS_CENTER_IP,REDIS_STATUS_PORT,pPeer->DevPort);
        redisAsyncCommand(buffev->owner->r_status,redis_op_status,buffev->owner,"EXPIRE %s %d",uuid.c_str(),REDIS_EXPIRE_TIME);
		pPeer->FlushRedis = nowtv.tv_sec;
	}

	/*响应设备*/
	std::stringstream timeout;
	timeout<<HEART_BEAT_TIMEOUT;
	Json::Value responseValue = Json::Value::null;
	responseValue["AgentProtocol"]["Body"]["KeepAliveIntervel"] = timeout.str();
	responseValue["AgentProtocol"]["Header"]["CSeq"] = "1";
	responseValue["AgentProtocol"]["Header"]["ErrorNum"] = "200";
	responseValue["AgentProtocol"]["Header"]["ErrorString"] = "Success OK";
	responseValue["AgentProtocol"]["Header"]["MessageType"] = DEV_REGIRSP;
	responseValue["AgentProtocol"]["Header"]["Version"] = "1.0";
	std::string strbody = getString(responseValue);
	int bodyLen = strbody.length();
	
	std::stringstream ss;
    ss<<bodyLen;
    std::string strLength = ss.str();
	std::string httphead = "HTTP/1.1 200 OK\r\n";
	std::string contenthead = "Content-Type: text/plain\r\n";
	std::string contentlength = "Content-Length: ";
	std::string headend = "\r\n\r\n";

	std::string strHeader = httphead + contenthead + contentlength + strLength + headend;
	std::string rspData = strHeader + strbody;

	int dataLen = rspData.length();
	bufferevent_write(buffev->bufev,rspData.c_str(),dataLen);
	ss.clear();
	timeout.clear();
	return 0;
}

/*处理设备连接函数*/
int handle_conn_requst(Buffev *buffev,std::string uuid,std::string token,std::string servertype,std::string sesionid,std::string destport)
{
	/*更新一下定时器*/
	update_timer_event(buffev->timer,HEART_BEAT_TIMEOUT);
	/*查找uuid*/
	Node * pPeer = get_node_from_map(uuid);
	if(NULL == pPeer)
	{
		return -1;
	}
	char match_ip[32] = {0,};
	int ret = get_matched_server(REDIS_CENTER_IP,(char *)servertype.c_str(),(char *)pPeer->Oemid.c_str(),(char *)pPeer->Area.c_str(),match_ip,NULL);
	if(ret < 0)
	{		
		return -1;	
	} 
	
	std::string strport = "6611";
	/*响应客户端*/
	Json::Value responseValue = Json::Value::null;
	responseValue["AgentProtocol"]["Header"]["Version"] = "1.0";
	responseValue["AgentProtocol"]["Header"]["CSeq"] = "3";
	responseValue["AgentProtocol"]["Header"]["MessageType"] = CLI_CONNRSP;	
	responseValue["AgentProtocol"]["Header"]["ErrorNum"] = "200";
	responseValue["AgentProtocol"]["Header"]["ErrorString"] = "Success OK";
	responseValue["AgentProtocol"]["Body"]["AgentServerIp"] = match_ip;
	responseValue["AgentProtocol"]["Body"]["AgentServerPort"] = strport.c_str();
	std::string strbody = getString(responseValue);
	int bodylen = strbody.length();

	std::stringstream ss;
    ss<<bodylen;
    std::string strLength = ss.str();
	std::string httphead = "HTTP/1.1 200 OK\r\n";
	std::string contenthead = "Content-Type: text/plain\r\n";
	std::string contentlength = "Content-Length: ";
	std::string headend = "\r\n\r\n";
	std::string strHeader = httphead + contenthead + contentlength + strLength + headend;
	std::string rspData = strHeader + strbody;

	int dataLen = rspData.length();
	bufferevent_write(buffev->bufev,rspData.c_str(),dataLen);
	ss.clear();
	
	/*向设备发送连接指令*/
	Json::Value reqConn = Json::Value::null;
	reqConn["AgentProtocol"]["Header"]["Version"] = "1.0";
	reqConn["AgentProtocol"]["Header"]["CSeq"] = "3";	
	reqConn["AgentProtocol"]["Header"]["MessageType"] = DEV_CONNREQ;	
	reqConn["AgentProtocol"]["Body"]["AgentServerIp"] = match_ip;
	reqConn["AgentProtocol"]["Body"]["AgentServerPort"] = strport;	
	reqConn["AgentProtocol"]["Body"]["SessionId"] = sesionid.c_str();
	reqConn["AgentProtocol"]["Body"]["DestPort"] = destport.c_str();
	reqConn["AgentProtocol"]["Body"]["ClientToken"] = token.c_str();
	std::string strreqCon = getString(reqConn);
	int reqbodylen = strreqCon.length();
	
	std::stringstream sl;
	sl<<reqbodylen;
	std::string strreqconnlen = sl.str();
	std::string reqConnHead = "POST / HTTP/1.1\r\n";
	std::string reqConnCseq = "CSeq: 3\r\n";
	std::string reqConnLen = "Content-Length: ";
	std::string reqConnend = "\r\n\r\n";
	std::string reqConnHeader = reqConnHead + reqConnCseq + reqConnLen + strreqconnlen + reqConnend;
	std::string reqConndata = reqConnHeader + strreqCon;
	int reqConndatalen = reqConndata.length();
	bufferevent_write(pPeer->WorBuf->bufev,reqConndata.c_str(),reqConndatalen);	
	sl.clear();
	return 0;
}
