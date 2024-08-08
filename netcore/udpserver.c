#include "net.h"

int udp_server_initial_func(const char *IP_Server, const char *Port_Server)
{
    //创建数据报套接字
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	printf("socket ok!\n");

	//绑定服务器地址
	Addr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));//清空地址结构
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((short)atoi(Port_Server));

    //判断地址类型
    if (0 == strcmp(IP_Server, BROADCAST))
    {
        //广播地址
        serverAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    }
    else if (0 == strcmp(IP_Server, MULTICAST))
    {
        //多播地址
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        //指定地址
	    serverAddr.sin_addr.s_addr = inet_addr(IP_Server);
    }

	if(bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		return SOCK_ERROR;
	}
	printf("套接字绑定成功\n");
    return sockfd;
}


/**
 * 函数功能：UDP服务端业务处理
 * 参数：1.sokcetfd
 * 返回值：成功返回0，失败返回失败代码
 * */
int udp_server_biz_process(const int sockfd)
{
    Addr_in clientAddr;
	bzero(&clientAddr, sizeof(clientAddr));
	int len = sizeof(clientAddr);
    int res = 0;
    char recv_buf[MAX_UDP_TRANSMIT_SIZE];
    //数据参数
	char *cmd[MAX_PARAMS_SIZE] = {0};
	char *fundID = NULL;

	while(1)
	{
		//接收客户端发送的业务
		if((res = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&clientAddr, &len)) < 0)
		{
			perror("read error");
			return RECV_ERROR;
		}
		else
		{
            //业务处理
            if ((res = message_convert(recv_buf, cmd, '#')) == OK)
            {
                
                //参数从第二个参数开始
                fundID = cmd[0];
                //查找功能路由执行任务
                if ((res = cmd_func_router(fundID, cmd, sockfd, UDPSERVER, NULL)) < 0)
                {
                    return res;
                }
            }
            else
            {
                printf("客户端透传消息：%s\n", recv_buf);
            }
			printf("recvfrom client 业务[%s]\tIP = [%s]\t port = [%d]\n", \
					recv_buf, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			//给客户端应答
			sendto(sockfd, "udp_server bussiness done!", strlen("udp_server bussiness done!"), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
			printf("sendto  ack_client ok!\n");
		}
	}

    return OK;
}

/**
 * 函数功能：广播服务端启动
 * 参数：1.服务IP  2.服务端口
 */
void *udp_broadcast_server_bootstrap(void *args)
{
    //定义接收缓冲区
    char recv_buf[MAX_UDP_TRANSMIT_SIZE];
    int count = 0, res = 0, sockfd;
    //接收参数
    char **udp_params = (char **) args;
    const char *IP_Server_boot = udp_params[0];
    const char *port_server_boot = udp_params[1];

    //创建数据报套接字
    if ((sockfd = udp_server_initial_func(BROADCAST, port_server_boot)) < 0)
    {
        perror("UDP服务初始化失败");
        pthread_exit("UDP服务初始化失败");
    }

    //接收客户端发送的业务
	if ((res = udp_server_biz_process(sockfd)) < 0)
    {
        error_code_show(res);
		pthread_exit("广播业务处理失败");
    }

    //关闭套接字
	close(sockfd);
	pthread_exit("UDP服务结束");
}

/**
 * 函数功能：多播服务端启动
 * 参数：1.服务IP  2.服务端口
 */
void *udp_multicast_server_bootstrap(void *args)
{
    //定义接收缓冲区
    char recv_buf[MAX_UDP_TRANSMIT_SIZE];
    int count = 0, res = 0, sockfd;
    //接收参数
    char **udp_params = (char **) args;
    const char *IP_Server_boot = udp_params[0];
    const char *port_server_boot = udp_params[1];

    //创建数据报套接字
    if ((sockfd = udp_server_initial_func(MULTICAST, port_server_boot)) < 0)
    {
        perror("UDP多播服务初始化失败");
        pthread_exit("UDP多播服务初始化失败");
    }

    //加入到多播组
	struct ip_mreq addr;
	bzero(&addr, sizeof(addr));

	//说清楚:将本机加入到指定的多播组中去
	addr.imr_multiaddr.s_addr = inet_addr(IP_Server_boot);
	addr.imr_interface.s_addr = htonl(INADDR_ANY);

	if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &addr, sizeof(addr)) < 0)
	{
		perror("add to multicastAddr error");
		pthread_exit("修改套接字属性失败");
	}
	printf("add to multicastAddr ok!\n");

    //接收客户端发送的业务
	if ((res = udp_server_biz_process(sockfd)) < 0)
    {
        error_code_show(res);
		pthread_exit("UDP服务业务处理失败");
    }
	//关闭套接字
	close(sockfd);
	pthread_exit("UDP多播服务结束");
}

/**
 * 函数功能：UDP服务端启动
 * 参数：1.服务IP  2.服务端口
 */
void *udp_server_bootstrap(void *args)
{
    //定义接收缓冲区
    char recv_buf[MAX_UDP_TRANSMIT_SIZE];
	int count = 0, res = 0, sockfd;
    //接收参数
    char **udp_params = (char **) args;
    const char *IP_Server_boot = udp_params[0];
    const char *port_server_boot = udp_params[1];

    //创建数据报套接字
    if ((sockfd = udp_server_initial_func(IP_Server_boot, port_server_boot)) < 0)
    {
        printf("UDP服务初始化失败\n");
		pthread_exit("UDP服务初始化失败");
    }

    //接收客户端发送的业务
	if ((res = udp_server_biz_process(sockfd)) < 0)
    {
        error_code_show(res);
		pthread_exit("UDP服务业务处理失败");
    }
	//关闭套接字
	close(sockfd);
	pthread_exit("UDP服务结束");
}