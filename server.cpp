/*************************************************************************
	> File Name: server.cpp
	> Author: 
	> Mail:  
	> Created Time: 2020年08月13日 星期四 09时59分23秒
 ************************************************************************/

#include<netinet/in.h>
#include<netinet/tcp.h>
#include<sys/types.h>
#include<sys/socket.h>
// SIGPIPE产生的原因：向已经关闭的socket write，第一次导致对端发送RST报文，第二次调用write方法，会生成SIGPIPE信号，导致进程退出。
void handle_sigpipe(){
	struct sigaaction sa;
	bzero(&sa,sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE, &sa, NULL))
		return;
}

int setSocketNonBlocking(int fd){
	int flag = fcntl(fd, F_GETFL, 0);
	if(flag == -1)
		return -1;
	flag |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flag)==-1)
		return -1;
	return 0;
}

void setSocketNodelay(int fd){
	int enable = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)& enable, sizeof(enable));
}

void setSocketNoLinger(int fd){
	struct linger linger_;
	linger_.l_onoff = 1;
	linger_.l_linger = 30;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char *)&linger_, sizeof(linger_));
}

int socket_bind_listen(int port)
{
	if(port < 0 || port > 65535) 
		return -1;
	int listenfd;
	if((listenfd = socket(AF_INET, SOCK_STEAM, 0)) == -1)
		return -1;

	int optval = 1;
　　// #include<sys/socket.h>
	// int setsockopt(int socket, int level, int option_name, const void *option_value,size_t option_len);
	// 2 arg-level:被设置的选项的级别，SOL_SOCKET-套接字级别上设置选项
	if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) ==-1 ){
		close(listenfd);
		return -1;
	}
	
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	if(bind(listenfd, (struct sockaddr*)&server_addr,sizeof(server_addr)) == -1){
		close(listenfd);
		return -1;
	}
	// listen(int sockfd, int backlog)
	// backlog : define the max length to which the queue of pending connect may grow.
	if(listen(listenfd, 2048) == -1){
		close(listenfd);
		return -1;
	}
}
