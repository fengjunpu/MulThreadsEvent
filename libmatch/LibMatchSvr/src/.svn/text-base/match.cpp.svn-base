#include "match.h"
#include "redis_wrap.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>	
#include <sys/shm.h>	
#include <sys/msg.h>	
#include <sys/types.h>
#include <linux/unistd.h>
#include <unistd.h>
#define gettid() syscall(__NR_gettid)

typedef std::map<std::string, std::map<std::string, std::string> > ServerMap;
typedef std::map<std::string, ServerMap> ServerTypeMap;
typedef std::map<std::string, ServerTypeMap> DataCenterMap;
typedef std::vector<std::string> StringVector;

static DataCenterMap s_all_dc;
static pthread_mutex_t s_lock;
static int s_init = 0;
static StringVector s_all_type;
static StringVector s_all_redis;
static int s_redis_port = 5141;

#define FIELD_OEM   "VendorName"
#define FIELD_AREA  "ServerArea"
#define FIELD_STATUS  "Status"

bool common_is_validip(char *pIP)
{
    int a,b,c,d;
    if(pIP == NULL)
        return false;
    a = 0;b = 0;c = 0;d = 0;
    int ret = sscanf(pIP,"%d.%d.%d.%d",&a,&b,&c,&d);
    if(ret != 4)
    {
            return false;
        }
    if((a==0)||(a==255))
    {
            return false;
        }
    return true;
}

StringVector &split(const std::string &s, char delim, StringVector &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

int dump_all_dc(DataCenterMap all_dc)
{
    for(DataCenterMap::iterator iter_dc=all_dc.begin(); iter_dc!=all_dc.end(); iter_dc++)
    {
        fprintf(stdout,"7 [libmatch] dc ip: %s\n", iter_dc->first.c_str());
        for(ServerTypeMap::iterator iter_type=iter_dc->second.begin(); iter_type!=iter_dc->second.end(); iter_type++)
        {
            fprintf(stdout,"7 [libmatch]\tservice: %s\n", iter_type->first.c_str());
            for(ServerMap::iterator iter_s=iter_type->second.begin(); iter_s!=iter_type->second.end(); iter_s++)
            {
                fprintf(stdout,"7 [libmatch]\t\tservice ip: %s\n", iter_s->first.c_str());
                for(std::map<std::string,std::string>::iterator iter=iter_s->second.begin(); iter!=iter_s->second.end(); iter++)
                {
                    fprintf(stdout,"7 [libmatch]\t\t%s:%s\n", iter->first.c_str(), iter->second.c_str());
                }
                fprintf(stdout,"7 [libmatch]\n");
            }
           fprintf(stdout,"7 [libmatch]\t\t\n\n");
        }
        fprintf(stdout,"7 [libmatch]\n##################################################\n\n");
    }
}

// force == 1 时，即使区域不匹配，也分配服务器
// force == 0 时，必需区域匹配，才能分配服务器
// 所有分配必需满足 oem 匹配
std::string select_in_servermap(ServerMap &servermap, std::string oem, std::string continent, std::string country, std::string province, int force)
{
    std::string select_ip;
    int minStatus = 10000000;
	StringVector general_servers;
    StringVector oem_matched_servers;
    StringVector continent_matched_servers;
    StringVector country_matched_servers;
    StringVector province_matched_servers;
    StringVector matched_servers;

    fprintf(stdout,"7 [libmatch]===> %s:%s:%s:%s:%d\n",oem.c_str(), continent.c_str(), country.c_str(), province.c_str(), force);
    for(ServerMap::iterator iter=servermap.begin(); iter!=servermap.end(); iter++)
    {
        // check field
        fprintf(stdout,"7 [libmatch]check %s ?\n", iter->first.c_str());
        if(iter->second.find(FIELD_OEM) == iter->second.end()
                || iter->second.find(FIELD_AREA) == iter->second.end()
                || iter->second.find(FIELD_STATUS) == iter->second.end())
        {
            fprintf(stdout,"7 [libmatch] invalid server: %s, can not find some field\n", iter->first.c_str());
            continue;
        }

		std::string oem_of_server = iter->second[FIELD_OEM];
		if(oem_of_server == "General")
		{
			general_servers.push_back(iter->first);
		}
		
		if(oem_of_server.find(oem) != std::string::npos)
        {
			oem_matched_servers.push_back(iter->first);
        }
    }

	if(oem_matched_servers.size() == 0)
	{
		oem_matched_servers = general_servers;
	}
	
    for(StringVector::iterator iter=oem_matched_servers.begin(); iter!=oem_matched_servers.end(); iter++)
    {
        if((!continent.empty()) && (servermap[*iter][FIELD_AREA].find(continent) != std::string::npos))
            continent_matched_servers.push_back(*iter);
        if((!country.empty()) && (servermap[*iter][FIELD_AREA].find(country) != std::string::npos))
            country_matched_servers.push_back(*iter);
        if((!province.empty()) && (servermap[*iter][FIELD_AREA].find(province) != std::string::npos))
            province_matched_servers.push_back(*iter);
    }

    if(province_matched_servers.size() > 0)
        matched_servers = province_matched_servers;
    else if(country_matched_servers.size() > 0)
        matched_servers = country_matched_servers;
    else if(continent_matched_servers.size() > 0)
        matched_servers = continent_matched_servers;
    else if(force)
        matched_servers = oem_matched_servers;
	if (matched_servers.size() <= 0)
	{
		fprintf(stdout,"7 [libmatch] can not find match server num: %d\n", matched_servers.size());
	}   
    minStatus = 10000000;
    for(StringVector::iterator iter=matched_servers.begin(); iter!=matched_servers.end(); iter++)
    {
        if(minStatus > atoi(servermap[*iter][FIELD_STATUS].c_str()))
        {
            minStatus = atoi(servermap[*iter][FIELD_STATUS].c_str());
            select_ip = *iter;
        }
    }

    if (!select_ip.empty())
    {
        std::stringstream ss;
        ss << ++minStatus;
        ss >> servermap[select_ip][FIELD_STATUS];
    }

    return select_ip;
}


int get_matched_server(char *in_redis, char *service,char *oem, char *area, char * matched_server, char *matched_redis)
{
    std::string tmp_server;
    std::string tmp_redis;

    if(service == NULL || oem == NULL || area == NULL)
    {
        fprintf(stdout,"7 [libmatch] invalid arg\n");
        return -1;
    }
	fprintf(stdout,"7 [libmatch] service=[%s],oem=[%s],area=[%s] \n",service,oem,area);
	fprintf(stdout,"7 [libmatch] pid=%d===========\n",(unsigned int)gettid());
	
    std::string continent,country,province;
    StringVector vect_area;
    split(std::string(area), ':', vect_area);
    if(vect_area.size() > 0)
        continent = vect_area[0];
    if(vect_area.size() > 1)
        country = vect_area[1];
    if(vect_area.size() > 2)
        province = vect_area[2];
	fprintf(stdout,"7 [libmatch] continent=[%s],country=[%s],province=[%s] \n",continent.c_str(),country.c_str(),province.c_str());
#if 0
    char *p, *q;
    p = strstr(area, ":");
    if(p != NULL)
    {
        continent = std::string(area, p-area);

        p += 1;
        q = strstr(p, ":");
        if(q != NULL)
        {
            country = std::string(p, q-p);
            province = std::string(q+1);
        }
    }
#endif

	fprintf(stdout,"7 [libmatch] dc size = %d \n",s_all_dc.size());		
    pthread_mutex_lock(&s_lock);
    for(DataCenterMap::iterator iter_dc=s_all_dc.begin(); iter_dc!=s_all_dc.end(); iter_dc++)
    {
        int force = 0;
        if(in_redis != NULL)
        {
        	force = 1;
            if(iter_dc->first != std::string(in_redis))
                continue;
        }
        fprintf(stdout,"7 [libmatch] dc: %s, force: %d\n", in_redis, force);
        if(iter_dc->second.find(std::string(service)) == iter_dc->second.end())
            continue;

        fprintf(stdout,"7 [libmatch] find service \n");

        ServerMap &servermap = iter_dc->second[std::string(service)];
        std::string select_ip = select_in_servermap(servermap, oem, continent,country,province, force);
        if(!select_ip.empty())
        {
            tmp_server = select_ip;
            tmp_redis = iter_dc->first;
            break;
        }
    }
    pthread_mutex_unlock(&s_lock);

    if(tmp_server.empty())
    {
        fprintf(stdout,"7 [libmatch] tmp server empty\n");
        return -1;
    }

    if(matched_redis != NULL)
        strcpy(matched_redis, tmp_redis.c_str());
    if(matched_server != NULL)
        strcpy(matched_server, tmp_server.c_str());
    return 0;
}

static int update_serverinfo_from_redis(char *redis_ip, int redis_port, char *service, ServerMap &oldmap, ServerMap &servermap)
{
    redisContext* connect = redis_connect(redis_ip, redis_port);
    const struct timeval tv = { 10, 0 };
    if (!connect || (redisSetTimeout(connect, tv) != REDIS_OK))
    {
        fprintf(stdout,"7 [libmatch] redis connect error [%s:%d]\n",redis_ip,redis_port);
		servermap = oldmap;
        return 0;
    } 

    std::map<std::string,std::string> tmp_total_map = redis_hgetall_map(connect, (std::string(service)+"Map").c_str());
    std::map<std::string,std::string>::iterator it = tmp_total_map.begin();
	if(tmp_total_map.size() == 0)
	{
		fprintf(stdout,"7 [libmatch] redis get service map error [%s:%d]\n",redis_ip,redis_port);
		redisFree(connect);
		servermap = oldmap;
		return 0;
	}

    for (; it != tmp_total_map.end(); ++it)
	{
        if(!common_is_validip((char*)it->first.c_str())
                || atoi(it->second.c_str()) == 0)
        {
            continue;
        }
        std::map<std::string, std::string> tmp_one_map;
        tmp_one_map = redis_hgetall_map(connect,
                std::string(std::string(service) + "_" + it->first).c_str());
		
		if(tmp_one_map.size() == 0)
		{
			fprintf(stdout,"7 [libmatch] redis get [%s_%s] status error [%s:%d], use the old status\n",service, it->first.c_str(), redis_ip,redis_port);
			if(oldmap.find(it->first) != oldmap.end())
			{
				tmp_one_map = oldmap[it->first];
			}
		}
		
		servermap[it->first] = tmp_one_map;
	}

    redisFree(connect);
    return 0;
}

int refresh_server_info(char *redis_ip, char *service)
{
    if(s_init == 0)
    {
        pthread_mutex_init(&s_lock, NULL);
		if(service != NULL)
		{
			split(service, '_', s_all_type);
		}
		else
		{
			s_all_type.push_back("AUTH");
			s_all_type.push_back("PMS");
			s_all_type.push_back("PIC");
			s_all_type.push_back("TPS");
			s_all_type.push_back("ALC");
			s_all_type.push_back("CFG");
			s_all_type.push_back("ReclaimAlarm");
			s_all_type.push_back("ReclaimPic");
			s_all_type.push_back("STAT");
			s_all_type.push_back("DSS");
			s_all_type.push_back("DSSPub");
			s_all_type.push_back("DSSStream");
			s_all_type.push_back("RpsAccess");
			s_all_type.push_back("RpsCmd");
			s_all_type.push_back("RpsVoIP");
			s_all_type.push_back("RpsAV");
		}

        s_init = 1;
    }

    if(redis_ip == NULL)
    {
		fprintf(stdout,"7 [libmatch] refresh_server_info error redis_ip=null\n");
		return -1;
	}   
    s_all_redis.clear();
    split(std::string(redis_ip), ':', s_all_redis);

	fprintf(stdout,"7 [libmatch] refresh_server_info redis_ip = [%s] \n",redis_ip);
	
    DataCenterMap tmp_all_dc;
    for (StringVector::iterator iter_ip=s_all_redis.begin(); iter_ip!=s_all_redis.end(); iter_ip++) {
        std::string dc_redis_ip = *iter_ip;
		fprintf(stdout,"7 [libmatch] dc: %s\n", dc_redis_ip.c_str());
		
        for(StringVector::iterator iter_type=s_all_type.begin(); iter_type!=s_all_type.end(); iter_type++)
        {
            std::string service = *iter_type;
            ServerMap servermap;
			ServerMap oldmap;
			fprintf(stdout,"7 [libmatch] service type: %s\n", service.c_str());
			
			if((s_all_dc.find(dc_redis_ip) != s_all_dc.end()) && (s_all_dc[dc_redis_ip].find(service) != s_all_dc[dc_redis_ip].end()))
			{
				oldmap = s_all_dc[dc_redis_ip][service];
			}
            update_serverinfo_from_redis((char *)dc_redis_ip.c_str(), s_redis_port, (char *)service.c_str(), oldmap, servermap);

            if(tmp_all_dc.find(dc_redis_ip) == tmp_all_dc.end())
            {
                ServerTypeMap empty;
                tmp_all_dc[dc_redis_ip] = empty;
            }
            if(tmp_all_dc[dc_redis_ip].find(service) == tmp_all_dc[dc_redis_ip].end())
            {
                ServerMap empty;
                tmp_all_dc[dc_redis_ip][service] = empty;
            }

            tmp_all_dc[dc_redis_ip][service].insert(servermap.begin(), servermap.end());
        }
    }

    pthread_mutex_lock(&s_lock);
    s_all_dc = tmp_all_dc;
    pthread_mutex_unlock(&s_lock);
	
	fprintf(stdout,"7 [libmatch] dc num: %d\n", s_all_dc.size());
	fprintf(stdout,"7 [libmatch] pid=%d===========\n",(unsigned int)gettid());
    dump_all_dc(tmp_all_dc);

    return 0;
}

