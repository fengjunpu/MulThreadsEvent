#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

int main()
{
	int fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6630);
	addr.sin_addr.s_addr = inet_addr("123.59.27.192");
	int flag = fcntl(fd,F_GETFL,0);
	fcntl(fd,F_SETFL,flag|O_NONBLOCK);


	struct sockaddr_in baddr;
	socklen_t blen = sizeof(baddr);
	baddr.sin_family = AF_INET;
	baddr.sin_port = htons(7760);
	baddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	bind(fd, (const struct sockaddr *)&baddr,blen);
	
	int ret = connect(fd,(const struct sockaddr *)&addr,len);
	if(ret < 0 && errno != EINPROGRESS)
	{
		printf("connect socket fd failed\n");
		return -1;
	}
	sleep(5);
	int optval = -1;
	socklen_t optlen = sizeof(optval);
	ret = getsockopt(fd,SOL_SOCKET,SO_ERROR,&optval,&optlen);
	if(ret == 0)
	{
		printf("connect sucess\n");
	}
	else
	{
		printf("get socket opt failed connect server failed\n");
	}
	while(1)
	{
		sleep(10);
	}
	return 0;
}
