#include "./net.h"

int client_initial_func(const char *IP_Client, const char *Port_Client)
{
	//创建套接字
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket error");
		return SOCK_ERROR;
	}
	printf("socket ok!\n");

	//定义地址信息结构，存储服务器的IP地址和端口号
	struct sockaddr_in serverAddr;
	//清空
	memset(&serverAddr, '\0', sizeof(serverAddr));
	//赋值 
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((short)atoi(Port_Client));
	serverAddr.sin_addr.s_addr = inet_addr(IP_Client);
	
	//发起连接请求
	int ret = connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if(ret < 0)
	{
		perror("connect error");
		return CONNECT_ERROR;
	}
	printf("connect ok!\n");

	return sockfd;
}


/**
 * 指令发送
 * 参数：1.sockfd 2.order指令 3.pSession会话信息
 * 返回值：成功返回OK，失败返回错误码
 */
int send_order(int sockfd, char *order, PSESSION pSession)
{
	//给服务器发送数据
	char *cmd[MAX_PARAMS_SIZE] = {0};
	//业务功能号
	char *funcID = NULL;
	int count = 0, res = 0;

	//发送服务端
	count = send(sockfd, order, strlen(order) + 1, 0);
	
	//解析业务功能号
	if ((res = message_convert(order, cmd, '#')) == 0)
	{
		printf("test\n");
		int i = 0;
		while(*(cmd + i) != NULL)
		{
			printf("%s ", *(cmd + i));
			i++;
		}
		//获取功能号
		funcID = cmd[0];
		//查找功能路由执行任务
		if ((res = cmd_func_router(funcID, cmd, sockfd, CLIENT, pSession)) < 0)
		{
			return res;
		}
	}
	
	if(count < 0)
	{
		perror("send error");
		return SEND_ERROR;
	}
	else if(0 == count)
	{
		printf("未发送任何消息!\n");
		return QUIT; 
	}
    else
	{
		printf("send to server ok!\n");
	}
	return OK;
}

int client_com_func(int sockfd)
{
	//给服务器发送数据
	char Message[MAX_TCP_TRANSMIT_SIZE] = {0};
	char *cmd[MAX_PARAMS_SIZE] = {0};
	//业务功能号
	char *funcID = NULL;
	int count = 0, res = 0;
    fflush(stdin);
	printf("请输入业务指令(格式#FUNCID#PARAM1#PARM2...):");
	scanf("%s%*c", Message);
	//单独判断Message内容是否为quit
	if(0 == strncasecmp("quit", Message, 4))//QUIT  Quit quit
	{
		return QUIT;
	}

	//发送服务端
	count = send(sockfd, Message, sizeof(Message), 0);
	
	//解析业务功能号
	if ((res = message_convert(Message, cmd, '#')) == 0)
	{
		int i = 0;
		while(*(cmd + i) != NULL)
		{
			printf("%s ", *(cmd + i));
			i++;
		}
		//获取功能号
		funcID = cmd[0];
		//查找功能路由执行任务
		if ((res = cmd_func_router(funcID, cmd, sockfd, CLIENT, NULL)) < 0)
		{
			return res;
		}
	}
	
	if(count < 0)
	{
		perror("send error");
		return SEND_ERROR;
	}
	else if(0 == count)
	{
		printf("未发送任何消息!\n");
		return QUIT; 
	}
    else
	{
		printf("send to server ok!\n");
	}
	return OK;
}

void *tcp_client_bootstrap(void *args)
{
	//搭建TCP客户端 
	char **tcp_params = (char **) args;
	const char *IP_Client_Boot = tcp_params[0];
	const char *port_client_boot = tcp_params[1];
	int sockfd = 0;
	int res = 0;

    //客户端端初始化
	while((sockfd = client_initial_func(IP_Client_Boot, port_client_boot)) < 0)
	{
		sleep(5);
	}
	printf("server_initial success!\n");

	//通信 
	while(1)
	{
		if((res = client_com_func(sockfd)) < 0)
		{
			error_code_show(res);
			break;
		}
	}

	//关闭套接字
	close(sockfd);
	printf("客户端线程结束\n");
	pthread_exit(NULL);
}
