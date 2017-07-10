#include "../include/server.h"
#include <cstdlib>
int main(int argc,char **argv)
{
	int n = 100;
	int index = 0;
	if(argc > 1)
	{
		n = atoi(argv[1]);
		index = atoi(argv[2]);
		std::cout<<"worker number: "<<n<<" start index: "<<index<<std::endl;
	}
	Server * Control_Button = Server::getInstance();
	Control_Button->Server_Start(5,n,index);
	std::string buf;
	std::cin>>buf;
	Control_Button->Server_Stop();
	return 0;
}
