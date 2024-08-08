#include "net.h"

/**
 * 函数功能：UDP客戶端初始化
 * 参数：IP_Server：服务器地址 
 * 参数：Port_Server：服务器端口号
 * 返回值：成功返回套接字描述符，失败返回错误码
 */
int udp_client_initial_func(const char *IP_Server, const char *Port_Server, const Addr_in *serverAddr)
{
    //创建数据报套接字
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	printf("socket ok!\n");

    //允许广播
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt error");
        return SET_OPTION_FAILED;
    }

	if(bind(sockfd, (struct sockaddr *)serverAddr, sizeof(Addr_in)) < 0)
	{
		return SOCK_ERROR;
	}
	printf("套接字绑定成功\n");
    return sockfd;
}

/**
 * 函数功能：UDP客戶端启动
 * 参数：IP_Client_Boot：服务器地址 
 * 参数：port_client_boot：服务器端口号
 */
void *udp_client_bootstrap(void *args)
{
    char **udp_params = (char **) args;
	const char *IP_Client_Boot = udp_params[0];
	const char *port_client_boot = udp_params[1];
	int sockfd = 0;
	int res = 0;

    //设置广播地址以及接收方自己的端口号  //serverAddr <==> broadcastAddr
	Addr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));//清空地址结构

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((short)atoi(port_client_boot));

    
    //判断地址类型
    if (0 == strcmp(IP_Client_Boot, BROADCAST))
    {
        //广播地址
        serverAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    }
    else if (0 == strcmp(IP_Client_Boot, MULTICAST))
    {
        //多播地址
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        //指定地址
	    serverAddr.sin_addr.s_addr = inet_addr(IP_Client_Boot);
    }

    //创建数据报套接字
    int count = 0;
    while ((sockfd = udp_client_initial_func(BROADCAST, port_client_boot, &serverAddr)) < 0)
    {   
        //等待建立一分钟
        if (count < 12)
        {
            perror("UDP服务初始化失败");
            pthread_exit("UDP服务初始化失败");
        }
        else
        {
            sleep(5);
        }
    }

    //发送数据
    //设置广播地址以及接收方自己的端口号  //serverAddr <==> broadcastAddr
}

/**
 * 函数功能：UDP客戶端业务处理
 * 参数：sockfd：套接字描述符 serverAddr:服务器地址 
 * 返回值：成功返回OK，失败返回错误码
 */
int udp_client_biz_process(const int sockfd, const Addr_in *serverAddr)
{
    char send_buf[MAX_UDP_TRANSMIT_SIZE];
	
	while(1)
	{
	    //发送业务指令给服务器
        memset(send_buf, 0, sizeof(send_buf));
        printf("请发送业务指令消息:\n");
        scanf("%s", send_buf);
        sendto(sockfd, send_buf, strlen(send_buf), 0, (struct sockaddr *)serverAddr, sizeof(Addr_in));

        printf("sendto broadcastAddr data  ok!\n");
        int count;
	    int len = sizeof(Addr_in);
		
		//处理服务器的应答
		int ret = recvfrom(sockfd, send_buf, sizeof(send_buf), 0, (struct sockaddr *)serverAddr, &len);
		printf("服务器应答:[%s]\t IP = [%s]\tport = [%d]\n", send_buf, inet_ntoa(serverAddr->sin_addr), ntohs(serverAddr->sin_port));
	}
	//关闭套接字
	close(sockfd);
	return OK;
}

