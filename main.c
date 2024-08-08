#include "./netcore/net.h"

//定义创建客户端和服务端的函数指针

int main(int argc, const char *argv[])
{
	const char *params[2] = {0};
	//定义发送和接收两个线程
	pthread_t pCli, pSrv;
	//客户端服务端线程返回参数
	char *sendThrStr, *recvThrStr;
	//判断有没有启动客户端和服务端线程
	int cliFlag = 0, svrFlag = 0;
	int res = 0;
	//判断传入的参数个数
	if (4 == argc || 7 == argc)
	{
		int count = 1;
		while(count < argc)
		{
			//根据参数判断客户端/服务端启动方式
			if (0 == strncasecmp("-thrs", argv[count], 5))
			{
				//创建服务端主线程(-thrs 代表多线程非阻塞模式)
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pSrv, NULL, tcp_server_bootstrap, (void *)params)) < 0)
				{
					perror("multithread服务端线程创建失败");
					exit(-1);
				}
				svrFlag = 1;
			}
			else if (0 == strcasecmp("-c", argv[count]))
			{
				//创建客户端线程（如果创建客户端统一用线程）
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pCli, NULL, tcp_client_bootstrap, (void *)params)) < 0)
				{
					perror("客户端线程创建失败");
					exit(-1);
				}
				cliFlag = 1;
			}
			else if (0 == strcasecmp("-udpc", argv[count]))
			{
				//创建UDP客户端线程（如果创建客户端统一用线程）
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pCli, NULL, udp_client_bootstrap, (void *)params)) < 0)
				{
					perror("客户端线程创建失败");
					exit(-1);
				}
				cliFlag = 1;
			}
			else if (0 == strcasecmp("-s", argv[count]))
			{
				//创建服务端主线程-s（默认为非阻塞IO模式）
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pSrv, NULL, tcp_epoll_server_bootstrap, (void *)params)) < 0)
				{
					perror("epoll服务端线程创建失败");
					exit(-1);
				}
				svrFlag = 1;
			}
			else if (0 == strncasecmp("-sels", argv[count], 5))
			{
				//创建服务端主线程(-thrs 代表多线程非阻塞模式)
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pSrv, NULL, tcp_select_server_bootstrap, (void *)params)) < 0)
				{
					perror("select服务端线程创建失败");
					exit(-1);
				}
				svrFlag = 1;
			}
			else if (0 == strncasecmp("-udps", argv[count], 5))
			{
				//创建udp服务端主线程
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pSrv, NULL, udp_server_bootstrap, (void *)params)) < 0)
				{
					perror("udp服务端线程创建失败");
					exit(-1);
				}
				svrFlag = 1;
			}
			else if (0 == strncasecmp("-bcst", argv[count], 5))
			{
				//创建广播服务端主线程
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pSrv, NULL, udp_broadcast_server_bootstrap, (void *)params)) < 0)
				{
					perror("服务端线程创建失败");
					exit(-1);
				}
				svrFlag = 1;
			}
			else if (0 == strncasecmp("-mcst", argv[count], 5))
			{
				//创建广播服务端主线程
                params[0] = argv[count + 1];
                params[1] = argv[count + 2];
				if ((res = pthread_create(&pSrv, NULL, udp_multicast_server_bootstrap, (void *)params)) < 0)
				{
					perror("多播服务端线程创建失败");
					exit(-1);
				}
				svrFlag = 1;
			}
			else 
			{
				printf("wrong params!\n");
				return ERROR;
			}
			
			//下一个参数（如果存在）
			count +=3;
		}
	}
	else
	{
		printf("wrong params!\n");
		return ERROR;
	}
	
	//主线程阻塞等待客户端、服务端线程结束
	if (1 == cliFlag)
	{
		pthread_join(pCli, (void **)&recvThrStr);
	}
	
	if (1 == svrFlag)
	{
		pthread_join(pSrv, (void **)&recvThrStr);
	}
	
	return OK;
}

