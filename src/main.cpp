#include "../include/server.h"
int main()
{
	Server * Control_Button = Server::getInstance();
	Control_Button->Server_Start(10,100000);
	std::string buf;
	std::cin>>buf;
	Control_Button->Server_Stop();
	return 0;
}
