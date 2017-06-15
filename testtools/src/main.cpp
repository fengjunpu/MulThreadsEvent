#include "../include/server.h"

int main(int argc,char **argv)
{
	Server * Control_Button = Server::getInstance();
	Control_Button->Server_Start(10,500);
	std::string buf;
	std::cin>>buf;
	Control_Button->Server_Stop();
	return 0;
}
