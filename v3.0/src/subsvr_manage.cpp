#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include "../include/subsvr_manage.h"
#include "../include/server.h"

static uint32_t COMMON_MINUTESECONDES = 360;

static int s_redis_update_count = 0;		
static int s_redis_exit = 0;
static int dss_server_port = 6610;
static int pull_thread_startflag = 0;
//通过环境变量获取配置
//和环境变量相关
static const char*  serverarea_name = NULL;
static const char*  vendor_name = NULL;
static const char*  server_type = NULL;
time_t begin_time = 0;

static char  rpsaccess_server_ip[48] = {0,};

static int  rediscenter_port = 5141;

static void update_serverinfo_to_redis(redisContext* connect)
{
	if (connect) 
	{
		if (0 == redis_multi(connect)) 
		{	
			std::string temp_type = server_type;
			std::string temp_map = temp_type + "Map";
			std::string rpsaccess_server_key(temp_type + "_"+std::string(rpsaccess_server_ip));
			std::string rpsaccess_server_port;
			std::string rpsaccess_server_status;
			char tps_run_time[48];
			
			time_t cur_time = ::time(NULL); 
			time_t up_time = cur_time - begin_time;
			sprintf(tps_run_time,"%d",(uint32_t)up_time);
			
			//获取当前服务连接数
			int peer_map_size = Server::getInstance()->Get_Node_Count();
			
			std::stringstream ss;
			ss << dss_server_port;
			ss >> rpsaccess_server_port;
			ss.clear();
			
			ss << peer_map_size;
			ss >> rpsaccess_server_status;

			int ret = redis_hset(connect,temp_map.c_str(),
							rpsaccess_server_ip, VALUE_DSSACCESSSERVER_ONLINE);
			
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_IP, rpsaccess_server_ip);
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_PORT, rpsaccess_server_port.c_str());
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_AREA, serverarea_name);
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_VENDORNAME, vendor_name);
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_STATUS,rpsaccess_server_status.c_str());
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_ACTIVEINDEX, rpsaccess_server_status.c_str());
							
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_RETOK, "0");
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_RETERROR,"0");
			ret += redis_hset(connect,rpsaccess_server_key.c_str(),
							FIELD_DSSACCESSSERVER_RUNSECONDS, tps_run_time);
			
			ret += redis_expire(connect,rpsaccess_server_key.c_str(), COMMON_MINUTESECONDES);
			if (ret < 0) 
			{
				redis_discard(connect);
			} 
			else 
			{
				redis_exec(connect);
			}
		}
	}
}

static int get_env_param()
{
	const char* temp_server_ip = getenv("RedisCenter");
	if(temp_server_ip != NULL)
	{
		strcpy(REDIS_CENTER_IP,temp_server_ip);
	}

	const char* temp_status_ip = getenv("RedisRpsStatus");
	if(temp_status_ip != NULL)
	{
		strcpy(REDIS_STATUS_IP,temp_status_ip);
	}

	serverarea_name = getenv("ServerArea");
	if(NULL == serverarea_name)
	{
		serverarea_name = "Asia:China:Beijing";
	}

	vendor_name = getenv("VendorName");
	if(NULL == vendor_name)
	{
		vendor_name = "General";
	}

	server_type = getenv("ServerType");
	if(NULL == server_type)
	{
		server_type = "RPS";
	}

	const char *tmp_server_iplist = getenv("RedisList");
	if(tmp_server_iplist != NULL)
	{
		strcpy(REDIS_CENTER_LIST_IP,tmp_server_iplist);
	}
	else
	{
		strcpy(REDIS_CENTER_LIST_IP,REDIS_CENTER_IP);
	}
	
	return 0;
}

static void* redis_update_thread(void* arg)
{
	while(!s_redis_exit)
	{	
		log_warnx("============>REDIS_CENTER_IP: %s,rediscenter_port = %d\n",REDIS_CENTER_IP,rediscenter_port);
		redisContext* connect = redis_connect(REDIS_CENTER_IP,rediscenter_port);
		if (connect) 
		{
			update_serverinfo_to_redis(connect);
			redisFree(connect);
			s_redis_update_count++;
			//sleep(COMMON_MINUTESECONDES/2);	
			sleep(30);
		}
		else
		{
			if(s_redis_update_count==0) 
			{
				sleep(1);
			}
			else
			{
				//sleep(COMMON_MINUTESECONDES);
				sleep(15);
			}
		}
	}
	return NULL;
}

static void* pull_serverinfo_thread(void* arg)
{
	//定期拉取服务信息
	int ret = 0;
	std::string sertype = "RpsCmd_RpsVoIP_RpsAV";
	pull_thread_startflag = 1;
	while(1)
	{
		log_warnx("============>REDIS_CENTER_LIST_IP: %s\n",REDIS_CENTER_LIST_IP);
		ret = refresh_server_info(REDIS_CENTER_LIST_IP,(char *)sertype.c_str());
		if(ret < 0)
		{
			sleep(3);
		}
		else
		{
			sleep(5);	
		}	
	}
	return NULL;
}

int	start_subsvr_manage(const char *m_server_ip,const char *m_cfgserver_ip)
{	
	begin_time = ::time(NULL);
	if(NULL != m_server_ip)
	{
		strcpy(rpsaccess_server_ip,m_server_ip);
	}
	else
	{
		return -1;
	}

	if(NULL != m_cfgserver_ip)
	{
		strcpy(REDIS_CENTER_IP,m_cfgserver_ip);
	}
	else
	{
		return -1;
	}
	
	pthread_t tid[2];
	pthread_attr_t attr[2];
	pthread_attr_init(&attr[0]);
	pthread_attr_init(&attr[1]);
	
	//创建线程，定期更新数据库中rpsaccess的状态信息
	pthread_create(&tid[0], &attr[0], redis_update_thread, NULL);
	
	//创建线程，定期拉取服务信息
	pthread_create(&tid[1], &attr[1], pull_serverinfo_thread, NULL);

	
	int cost = 0;
	while (((s_redis_update_count <= 0)||(pull_thread_startflag<=0))&&(cost < 20))
	{
		log_warnx("wait for redis_update");
		sleep(1);	
		cost++;	
	}
	
	if(cost >= 20)
	{
		log_errx("wait for redis_update timeout");
		return -1;
	}
	
	return 0;
}

int	stop_subsvr_manage()
{
	s_redis_exit = 1;
	sleep(1);	
	return 0;
}

int get_param()
{
	//获取环境变量
	get_env_param();
	std::ifstream fs("Status_RedisIp.txt");
	if(fs)
	{
		char line[1024] = {0,};
		while(fs.getline(line,sizeof(line)))
		{
			std::stringstream ss;
			ss.str(line);
			std::string item;
			int i = 0;
			if(std::getline(ss,item,':'))
			{
				if(item == "status")
				{
					if(std::getline(ss,item,':'))
					{
						if(strlen(item.c_str()) > 0)
							strcpy(REDIS_STATUS_IP,item.c_str());
					}
				}
				else if(item == "center")
				{
					if(std::getline(ss,item,':'))
					{
						if(strlen(item.c_str()) > 0)
							strcpy(REDIS_CENTER_IP,item.c_str());
					}
				}
				else if(item == "centerlist")
				{	
					if(std::getline(ss,item,':'))
					{
						if(strlen(item.c_str()) > 0)
							strcpy(REDIS_CENTER_LIST_IP,item.c_str());
					}
				}
			}
		}
		fs.close();
	}
	log_warnx("=====>REDIS_STATUS_IP:%s,REDIS_CENTER_IP = %s,REDIS_CENTER_LIST_IP = %s",REDIS_STATUS_IP,REDIS_CENTER_IP,REDIS_CENTER_LIST_IP);
	
	return 0;
}
