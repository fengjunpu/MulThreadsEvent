#include "../include/server.h"
#include "../include/subsvr_manage.h"
#include <cstring>
#include<unistd.h>
#include <signal.h>

static const char * optstr = "hi:s:a:p:u:c:v:t:e:r:";
static const char * help   =	"Options: \n"	"  -h         : This help text\n"	
								"  -i 	<str> : Local Http Server IP \n"	
								"  -s 	<int> : Local Http Server Port \n"	
								"  -a 	<str> : Status Redis IP\n"
								"  -p 	<int> : Status Redis Port\n"
								"  -u 	<str> : Data center ip"
								"  -v 	<int> : Peer Keepalive Interval\n"	
								"  -t 	<int> : Peer Keepalive Timeout\n"	
								"  -e	<int> : Redis ExpireTime\n"
								"  -r	<int> : Redis Relink Time\n"
								"  -c	<int> : Reduis Auth Port\n"; 

static int parse_args(int argc, char ** argv) 
{	
	extern char * optarg;	
	int           c;	
	while ((c = getopt(argc, argv, optstr)) != -1) 
	{	
		switch (c) 
		{	        
			case 'h':	            
			std::cout<<"Usage: "<<argv[0]<<"[opts] "<<help<<std::endl;	            
			return -1;	        
			case 'i':	            
			memcpy(RPS_SERVER_IP,strdup(optarg),sizeof(RPS_SERVER_IP));
			break;	        
			case 's':	            
			RPS_SERVER_PORT = atoi(optarg);	            
			break;	
			case 'a':
			memcpy(REDIS_CENTER_IP,strdup(optarg),sizeof(REDIS_CENTER_IP));		
			break;
			case 'p':
			REDIS_STATUS_PORT = atoi(optarg);	
			break;
			case 'u':
			memcpy(REDIS_CENTER_IP,strdup(optarg),sizeof(REDIS_CENTER_IP));
			break;
			case 'c':
			REDIS_AUTH_PORT = atoi(optarg);
			break;
			case 'v':	            
			HEATER_BEAT_INTERNAL = atoi(optarg);	            
			break;
			case 't':	            
			HEART_BEAT_TIMEOUT = atoi(optarg);	            
			break;
			case 'e':	            
			REDIS_EXPIRE_TIME = atoi(optarg);	            
			break;
			case 'r':	            
			REDIS_RECONN_INTERNAL = atoi(optarg);	            
			break;	          
			default:	            
			std::cout<<"Unknown opt "<<optarg<<std::endl;	            
			return -1;	
		}	
	}	
	return 0;
}

int main(int argc,char **argv)
{
	//½âÎö²ÎÊý 
	signal(SIGPIPE,SIG_IGN);
	parse_args(argc,argv);
	int Ret = get_param(REDIS_CENTER_IP);
	assert(Ret == 0);
	if((strlen(REDIS_CENTER_IP)==0) || (strlen(RPS_SERVER_IP)==0))
	{
		return -1;
	}
 	Ret = start_subsvr_manage(RPS_SERVER_IP,REDIS_CENTER_IP);
	assert(Ret == 0);
	
	std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!begin!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	Server * Control_Button = Server::getInstance();	
	Control_Button->Server_Start(1);
	Control_Button->Server_Stop();
	std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!end!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	return 0;
}
