#include "TcpServer.h"
#include "base/utility.h"
#include "httpData.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
using namespace std;

server::server(EventLoop *loop, int port, int threadnum):
				loop_(loop),
				pool_(new EventLoopThreadPool(loop_, threadnum)),
				port_(port),
				listenfd_(socket_bind_listen(port_)),
				acceptChannel_(new Channel(loop_, listenfd_)),
				threadnum_(threadnum),
				mutex_()
{
	if(setSocketNonBlocking(listenfd_) < 0)
		perror("listenfd_ set non block failed!");
	//handle_for_sigpipe();
}
void server::start()
{
	pool_->start();
	acceptChannel_->setEvents(EPOLLIN);
	acceptChannel_->setReadResponse(bind(&server::handleNewConnection, this));
	//acceptChannel_->setConnResponse(bind(&server::handleThisConnection, this));

	loop_->addToPoller(acceptChannel_, 0);
}

server::~server()
{

}
void server::handleNewConnection()
{

	struct sockaddr_in client_addr = { 0 };
	socklen_t client_addr_len = sizeof client_addr;
	int accept_fd = 0;
	while((accept_fd = accept(listenfd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0){
		EventLoop *curLoop = pool_->getNextLoop();
		cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << " fd = " << accept_fd << endl;

		// 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }
		
		if(setSocketNonBlocking(accept_fd) < 0){
			perror("set non block failed!");
		}		
		setSocketNodelay(accept_fd);
		setSocketNoLinger(accept_fd);

		spHttpData accept_http(new httpData(curLoop, accept_fd));
		accept_http->getChannel()->setHolder(accept_http);
		curLoop->queueInLoop(bind(&httpData::handleNewConn, accept_http));	//加入到pendingFuntors
	}


}

