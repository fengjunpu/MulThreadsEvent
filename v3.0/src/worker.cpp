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


template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

int parse_http_msg(char *pbuf, int len, http_msg_t *phttp_msg)
{
    char *pbegin = strstr(pbuf, "POST");
    if(pbegin == NULL)
        pbegin = strstr(pbuf, "PUT");
    if(pbegin == NULL)
        pbegin = strstr(pbuf, "GET");
    if(pbegin == NULL)
        pbegin = strstr(pbuf, "HTTP");
    if(pbegin == NULL)
        return 0;

    int prefix_len = pbegin - pbuf;
    char *p = strstr(pbegin, "\r\n\r\n");
    if(p == NULL)
        return 0;
    char *pbody = p+4;
    int headlen = pbody-pbegin;

    vect_str_t head_line_vect;
    char *ptmp = strstr(pbegin, "\r\n");
    std::string strtmp(pbegin, ptmp-pbegin);
    head_line_vect = split(strtmp, ' ');
    if(head_line_vect.size() < 2)
        return -1;

    if(head_line_vect[0] == "HTTP/1.1")
    {
        phttp_msg->type = "response";
        phttp_msg->status_code = atoi(head_line_vect[1].c_str());
    }
    else
    {
        phttp_msg->type = "request";
        phttp_msg->url = head_line_vect[1];
    }

    std::string strheader(pbegin, headlen);
    std::istringstream ssheader(strheader);
    std::string oneline;
    std::string::size_type index;

    while (std::getline(ssheader, oneline) && oneline != "\r")
    {
        index = oneline.find(':', 0);
        if(index != std::string::npos) {
            std::string key = oneline.substr(0, index);
            std::string value = oneline.substr(index + 1);
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            phttp_msg->headers[key] = value;
        }
    }

    if(phttp_msg->headers.find("content-length") == phttp_msg->headers.end())
        return -1;

    int bodylen = atoi(phttp_msg->headers["content-length"].c_str());
    if(bodylen + headlen > len-prefix_len)   
        return 0;

    phttp_msg->body = pbody;
    phttp_msg->bodylen = bodylen;
    phttp_msg->msglen = headlen + bodylen + prefix_len;
    return 1;
}

static std::string getString(const Json::Value & table)
{
    std::string temp;
    Json::StyledWriter writer(temp);
    writer.write(table);
    return temp;
}

int error_rps_data(struct bufferevent *bufev,int code)
{
	if(NULL == bufev)
		return -1;
	char rps[128] = {0};
	int ret = snprintf(rps,128,"HTTP/1.1 %d %s\r\n",code,status_code_to_str(code));
	sprintf(rps+ret,"%s","Content-Type: text/plain\r\n\r\n");
	int len = strlen(rps) + 1;
	bufferevent_write(bufev,rps,len);
	return 0;
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
	//std::cout<<"insert new peer uuid :"<<uuid<<std::endl;
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
int handle_client_rquest(http_msg_t *msg,Conn *buffev)
{
	if(NULL == msg)
		return -1;
	
	Json::Reader    reader;
    Json::Value     requestValue;
	if(reader.parse(msg->body, requestValue) == false)
   	{
		return error_rps_data(buffev->bufev,HTTP_RES_400);
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
			std::string StrPort = Ret ? requestValue["AgentProtocol"]["Body"]["DevicePort"].asCString() : "-1";
			
			return handle_dev_register(buffev,Uuid,Area,OemId,StrPort);
		}
		else if(CLI_CONNQEQ == MessageType)
		{
			bool Ret = false;
			do
			{
				if( !requestValue["AgentProtocol"]["Body"].isMember("ClientToken") || \
					!requestValue["AgentProtocol"]["Body"].isMember("ServiceType") || \
					!requestValue["AgentProtocol"]["Body"].isMember("SessionId") ||  \
					!requestValue["AgentProtocol"]["Body"].isMember("DestPort") )
				{
					break;
				}
				std::string ClientToken = requestValue["AgentProtocol"]["Body"]["ClientToken"].asCString();
				std::string ServerType = requestValue["AgentProtocol"]["Body"]["ServiceType"].asString();
				std::string SessionId = requestValue["AgentProtocol"]["Body"]["SessionId"].asCString();
				std::string DestPort = requestValue["AgentProtocol"]["Body"]["DestPort"].asCString();
				//std::cout<<"client uuid "<<Uuid<<std::endl;
				return handle_conn_requst(buffev,Uuid,ClientToken,ServerType,SessionId,DestPort);
			}while(0);
		}
	}
	
	/*Bad Request ... ...*/
	return error_rps_data(buffev->bufev,HTTP_RES_400);
}


/*处理设备注册请求*/
int handle_dev_register(Conn *conn,std::string uuid,std::string are,std::string oenmid,std::string port)
{
	Node * pPeer = get_node_from_map(uuid);
	if((NULL == pPeer) || (pPeer->pConn != conn))
	{
		int flag = 0;
		if(NULL == pPeer)
		{
			pPeer = (Node *)calloc(sizeof(Node),1);
			assert(pPeer);
			flag = 1;
		}
		else if(pPeer->pConn != NULL)	//free old connection
		{
			if(pPeer->pConn->timer != NULL)
			{
				
				if(event_initialized(pPeer->pConn->timer))
				{
					event_del(pPeer->pConn->timer);
				}
				event_free(pPeer->pConn->timer);
				pPeer->pConn->timer = NULL;
			}
			if(pPeer->pConn->bufev != NULL)
			{
				bufferevent_setcb(pPeer->pConn->bufev, NULL, NULL, NULL, NULL);
				bufferevent_free(pPeer->pConn->bufev);
			}
			free(pPeer->pConn);	
		}
		
		pPeer->FlushRedis = 0;
		pPeer->pConn = conn;
		pPeer->DevPort = atoi(port.c_str());
		memcpy(pPeer->Area,are.c_str(),sizeof(pPeer->Area));
		memcpy(pPeer->Oemid,oenmid.c_str(),sizeof(pPeer->Oemid));
		memcpy(pPeer->Uuid,uuid.c_str(),sizeof(pPeer->Uuid));
		pPeer->pConn->pPeer = pPeer;
		
		if(1 == flag)
			insert_node_to_map(uuid,pPeer);
	}
	
	update_timer_event(conn->timer,HEART_BEAT_TIMEOUT);
	
	/*更新数据库*/
	struct timeval nowtv;
	evutil_gettimeofday(&nowtv, NULL);
	if(conn->hredis->redis_conn_flag == 1 &&(nowtv.tv_sec - pPeer->FlushRedis > 3*HEATER_BEAT_INTERNAL - 5))
	{
		redisAsyncCommand(conn->hredis->r_status,redis_op_status,conn->hredis, "HMSET %s ServerIP %s ServerPort %d DevicePort %d", \
                                        uuid.c_str(),RPS_SERVER_IP,RPS_SERVER_PORT,pPeer->DevPort);
        redisAsyncCommand(conn->hredis->r_status,redis_op_status,conn->hredis,"EXPIRE %s %d",uuid.c_str(),REDIS_EXPIRE_TIME);
		pPeer->FlushRedis = nowtv.tv_sec;
	}

	/*响应设备*/
	std::stringstream timeout;
	timeout<<HEATER_BEAT_INTERNAL;
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
	bufferevent_write(conn->bufev,rspData.c_str(),dataLen);
	ss.clear();
	timeout.clear();
	return 0;
}

/*处理设备连接请求*/
int handle_conn_requst(Conn *conn,std::string uuid,std::string token,std::string servertype,std::string sesionid,std::string destport)
{
	update_timer_event(conn->timer,HEART_BEAT_TIMEOUT - 100);
	/*查找uuid*/
	Node * pPeer = get_node_from_map(uuid);
	if(NULL == pPeer)
	{
		return error_rps_data(conn->bufev,HTTP_RES_NOTFOUND);
	}
	
	char match_ip[32] = {0,};
	int ret = get_matched_server(REDIS_CENTER_IP,(char *)servertype.c_str(),(char *)pPeer->Oemid,(char *)pPeer->Area,match_ip,NULL);
	if(ret < 0)
	{		
		return error_rps_data(conn->bufev,HTTP_RES_SERVERR);
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
	bufferevent_write(conn->bufev,rspData.c_str(),dataLen);
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
	bufferevent_write(pPeer->pConn->bufev,reqConndata.c_str(),reqConndatalen);	
	sl.clear();
	return 0;
}
