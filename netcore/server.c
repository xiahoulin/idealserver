#include "./net.h"

int server_initial_func(const char *IP_Server, const char *Port_Server)
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
	serverAddr.sin_port = htons((short)atoi(Port_Server));
	serverAddr.sin_addr.s_addr = inet_addr(IP_Server);
	//绑定IP地址和端口
	if(bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind error");
		return BIND_ERROR;
	}

	//创建监听队列 
	if(listen(sockfd, 5) < 0)
	{
		perror("listen error");
		return LISTEN_ERROR;
	}
	printf("listening......\n");

	return sockfd;
}

int server_com_func(int newfd, PSESSION pSession)
{
	//接收客户端发送的数据 
	char Message[MAX_TCP_TRANSMIT_SIZE] = {0};
	//数据参数
	char *cmd[MAX_PARAMS_SIZE] = {0};
	char *fundID = NULL;
	//解析的数据
	char *res_data[MAX_PARAMS_SIZE] = {0};
	int res = 0, count = 0;
	PSESSION pCliSession = NULL;
	//查找session
	if (pSession != NULL){
		pCliSession = find_session(pSession, newfd);
	}

	if ((count = recv(newfd, Message, sizeof(Message), 0)) < 0)
	{
		perror("recv error");
		return RECV_ERROR;
	}
	
	if (count == 0)
	{
		printf("客户端已退出\n");
		return QUIT;
	}
	else
	{
		//业务处理
		if ((res = message_convert(Message, cmd, '#')) == OK)
		{
			
			//参数从第二个参数开始
			fundID = cmd[0];
			//查找功能路由执行任务
			if ((res = cmd_func_router(fundID, cmd, newfd, SERVER, pSession)) < 0)
			{
				return res;
			}
		}
		else
		{
			printf("客户端透传消息：%s\n", Message);
		}
	}
	return OK;
}

//启动服务端函数
void *tcp_server_bootstrap(void *args)
{
	//搭建TCP服务器 
	char **tcp_params = (char **) args;
	const char *IP_Server_boot = tcp_params[0];
    const char *port_server_boot = tcp_params[1];
    int res = 0;
	int newfd = 0;

    //服务器端初始化
	int sockfd = server_initial_func(IP_Server_boot, port_server_boot);
	if(sockfd < 0)
	{
		perror("服务端初始化失败");
		pthread_exit("服务初始化失败");
	}
	printf("server_initial success!\n");

	//等待建立连接（本质：获取连接成功的客户端对应的通信套接字，从内核中）
	struct sockaddr_in clientAddr;
	memset(&clientAddr, '\0', sizeof(clientAddr));
	int len = sizeof(clientAddr);
	while(1)
	{
		newfd = accept(sockfd, (struct sockaddr *)&clientAddr, &len);
		if(newfd < 0){
			perror("accept error");
			break; 
		}
		//创建线程
		pthread_t tid;
		pthread_create(&tid, NULL, tcp_server_t, &newfd);
		pthread_detach(tid);
		printf("新客户端newfd = %d accept ok!\tIP = %s\t Port = %d\n", newfd, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		tid = 0;
	}
	//关闭套接字
	close(sockfd);
	pthread_exit("服务端主程结束");
}

void *tcp_server_t(void *args)
{
	//获取套接字
	int newfd = *((int *)args);
	int res = 0;
	//通信 
	while(1)
	{
		if((res = server_com_func(newfd, NULL)) < 0)
		{
            error_code_show(res);
			break;
		}
	}
	close(newfd);
	pthread_exit("服务端线程结束");
}

/**
 * 单线程epoll方式启动服务端
 */
void *tcp_epoll_server_bootstrap(void *args)
{
	int sockfd = 0;
	int res = 0;
	char **tcp_params = (char **) args;
	const char *IP_Server_boot = tcp_params[0];
    const char *port_server_boot = tcp_params[1];
	PSESSION pSession = NULL;

	//服务器端初始化
	sockfd = server_initial_func(IP_Server_boot, port_server_boot);
	if (sockfd < 0)
	{
		perror("服务端初始化失败");
		pthread_exit("服务初始化失败");
	}
	printf("server_initial success\n");

	int epfd = epoll_create1(0);
	if (-1 == epfd)
	{
		perror("epoll_create error");
		close(sockfd);
		pthread_exit("epoll create error");
	}
	printf("epoll_create OK\n");

	//定义EPOLL事件结构体变量
	struct epoll_event event;
	event.events = EPOLLIN;
	//测试边缘触发
	//fcntl(sockfd, F_SETFL, O_NONBLOCK);
	//event.events |= EPOLLET;
	event.data.fd = sockfd;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) == -1)
	{
		perror("add listen error");
		close(sockfd);
		close(epfd);
		pthread_exit("epoll create error");
	}
	printf("add sockfd success\n");
	//创建session
	pSession = init_session();
	if (pSession == NULL)
	{
		perror("init session error");
		close(sockfd);
		close(epfd);
		pthread_exit("session init error");
	}

	//进入事件循环
	struct epoll_event events[MAX_EPOLL_EVENTS];
	//定义保存成功的新客户端的地址信息
	struct sockaddr_in newClientAddr;
	memset(&newClientAddr,  0, sizeof(newClientAddr));
	int len_new_client = sizeof(newClientAddr);
	
	//存储每一个连接成功的客户端套接字
	int newfd;
	while(1)
	{
		//等待就绪事件发生
		int ready_events_num = epoll_wait(epfd, events, MAX_EPOLL_EVENTS, -1);

		//遍历就绪事件数组
		int i;
		for (i = 0; i < ready_events_num; i++)
		{
			//判断是否为该监听事件的套接字
			if (sockfd == events[i].data.fd)
			{
				//新的套接字请求
				newfd = accept(events[i].data.fd, (struct sockaddr *)&newClientAddr, &len_new_client);
				if (newfd < 0)
				{
					perror("accept new client error");
					break;
				}
				printf("新客户端newfd = %d accept ok!\tIP = %s\t Port = %d\n", newfd, inet_ntoa(newClientAddr.sin_addr), ntohs(newClientAddr.sin_port));

				//将新的newfd记录在epoll中
				event.events = EPOLLIN;
				event.data.fd = newfd;

				//添加记录
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &event) == -1)
				{
					perror("add new clientfd error");
					break;
				}
				printf("注册新clientfd = %d到epoll中成功\n", newfd);

				//将新客户端加入到session中
				//添加session
				PSESSION newSession = NULL;
				if ((res = create_update_socket_session(sockfd, newfd, 0, NULL, NULL, &newSession)) < 0)
				{
					perror("add new client to session error");
					break; 
				}
				if (newSession == NULL) printf("newSession is null \n");
				if (add_session(pSession, newSession) < 0)
				{
					perror("add new client to session error");
					break; 
				}
				printf("添加新clientfd = %d到session中成功\n", newfd);

			}
			else
			{
				//找到当前套接字的session
				PSESSION cliSession = find_session(pSession, events[i].data.fd);
				//业务处理请求
				if((res = server_com_func(events[i].data.fd, cliSession)) < 0)
				{
           			error_code_show(res);
					//移除该client的套接字描述符
					if (epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]) < 0)
					{
						perror("client fd delete error\n");
						break;
					}
					printf("删除客户端clientfd = %d\n", events[i].data.fd);

					//从session中移除该client
					if (remove_session(pSession, events[i].data.fd) < 0)
					{
						perror("client fd remove error\n");
					}
					printf("从session中删除客户端clientfd = %d\n", events[i].data.fd);
					//关闭套接字
					close(events[i].data.fd);
				}
				 
			}
		}
	}

	//释放session
	destroy_session(&pSession);
	close(sockfd);
	close(epfd);	
	printf("服务结束\n");
	pthread_exit("服务结束");
}

/** 
 * 单线程方式启动服务端(引入sesson管理)
 */
void *tcp_select_server_bootstrap(void *args)
{
	int sockfd;
	int res = 0;
	int readyfd;
	char **tcp_params = (char **) args;
	const char *IP_Server_boot = tcp_params[0];
    const char *port_server_boot = tcp_params[1];
	fd_set readfds;
	PSESSION pSession = NULL;

	//服务器端初始化
	sockfd = server_initial_func(IP_Server_boot, port_server_boot);
	if (sockfd < 0)
	{
		perror("服务端初始化失败");
		pthread_exit("服务初始化失败");
	}
	printf("server_initial success\n");

	// 初始化文件描述符集合
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    int max_sd = sockfd;

	//定义保存成功的新客户端的地址信息
	struct sockaddr_in newClientAddr;
	memset(&newClientAddr,  0, sizeof(newClientAddr));
	int len_new_client = sizeof(newClientAddr);

	//存储每一个连接成功的客户端套接字
	int newfd;
	//进入事件循环
	while (1)
	{
		// 每次循环开始时，重新初始化 readfds
        fd_set rfds = readfds;

		newfd = -1;

		readyfd = select(max_sd + 1, &rfds, NULL, NULL, NULL);
		if (readyfd < 0)
		{
			perror("select error");
			break;
		}
		else if (readyfd == 0)
		{
			printf("select timeout\n");
			continue;
		}

		if (FD_ISSET(sockfd, &rfds)) {
            //新的套接字请求
				newfd = accept(sockfd, (struct sockaddr *)&newClientAddr, &len_new_client);
				if (newfd < 0)
				{
					perror("accept new client error");
					continue; // 改为continue，避免直接退出循环
				}
				printf("新客户端newfd = %d accept ok!\tIP = %s\t Port = %d\n", newfd, inet_ntoa(newClientAddr.sin_addr), ntohs(newClientAddr.sin_port));
				


				//将新的newfd记录在select集合中
				FD_SET(newfd, &readfds);

				// 更新 max_sd 如果需要
				if (newfd > max_sd) {
					max_sd = newfd;
				}
				printf("注册新clientfd = %d到select集合中成功\n", newfd);
        }
		
		printf("select readyfd = %d sockfd = %d max_sd = %d\n", readyfd, sockfd, max_sd);
		// 检查是否有客户端数据可读
		int i;
        for (i = sockfd + 1; i <= max_sd; i++) {
			//在select集合中, 且不是刚加入的newfd, 才处理
            if (FD_ISSET(i, &rfds) && i != newfd) {
                // 业务处理请求
				printf("select 客户端clientfd = %d\n", i);
                if ((res = server_com_func(i, NULL)) < 0) {
                    error_code_show(res);
                    // 关闭套接字并从select集合中移除
                    FD_CLR(i, &readfds);
                    close(i);
                    if (i == max_sd) {
                        // 重新计算max_sd
                        while (max_sd >= 0 && !FD_ISSET(max_sd, &rfds)) {
                            max_sd--;
                        }
                    }
                }
				printf("select 客户端clientfd = %d over\n", i);
            }
        }


	}
	
	close(sockfd);
	FD_ZERO(&readfds);
	printf("服务结束\n");
	pthread_exit("服务结束");
}

int download_file_send(PSESSION pSession, int sockfd, void *args)
{
	char **p_args = (char **)args;
	char *send_file_path = p_args[1];
	char send_buf[MAX_TCP_TRANSMIT_SIZE] = {0};
	off_t file_length = 0;
	int fd = 0, res = 0, readbyte = 0;

	//打开文件的信息
	struct stat fileInfo;

	// 获取文件状态信息
    if (lstat(send_file_path, &fileInfo) == -1) 
	{
        perror("获取文件状态信息失败");
        close(fd);
        return CHECK_STAT_FAILED;
    }

	//发送给客户端文件大小
	file_length = fileInfo.st_size;
	sprintf(send_buf, "%ld", file_length);
	ssize_t bytes_sent = send(sockfd, send_buf, sizeof(send_buf), 0);
    if (bytes_sent == -1) 
	{
        perror("Error sending data");
        return SEND_ERROR;
    }

	//服务端打开文件
	if ((fd = open(send_file_path, O_RDONLY)) < 0)
	{
		perror("打开文件失败");
		close(fd);
		return OPEN_FAILED;
	}

	//循环读取文件并发送
    memset(send_buf, 0, sizeof(send_buf));
	while((readbyte = read(fd, send_buf, MAX_TCP_TRANSMIT_SIZE)) >= 0)
	{
        if (readbyte == 0)
        {
            //读取到文件末尾
            printf("上传成功!\n");
            break;
        }
        
		bytes_sent = send(sockfd, send_buf, readbyte, 0);
		if (bytes_sent == -1)
		{
			perror("Error sending data");
			return SEND_ERROR;
		}
		printf("发送%ld个字节数据\n", bytes_sent);
		memset(send_buf, 0, sizeof(send_buf));
	}
	close(fd);
	
	return OK;
}

int download_file_receive(PSESSION pSession, int sockfd, void *args)
{
	//接收缓冲区
	char recv_buf[MAX_TCP_TRANSMIT_SIZE] = {0};
	char **p_args = (char **)args;
	char *receive_file_path = p_args[2];

	//获取文件的大小
	char file_length_str[10];
	long file_length = 0;
	int fd = 0;
	ssize_t write_bytes = 0;

	ssize_t recv_bytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0);

    if (recv_bytes == -1) {
        perror("Error receiving data");
        return RECV_ERROR;
    }
	
	file_length = atol(recv_buf);
	printf("接收的文件大小:%ld bytes", file_length);
    memset(recv_buf, 0, sizeof(recv_buf));
    
    //文件存在才接收
    if (0 != file_length)
    {
        //只写的方式打开文件
        if ((fd = open(receive_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
        {
            perror("打开文件失败");
            close(fd);
            return OPEN_FAILED;
        }
        
        //如果不是完整的缓冲区则持续接收
        while((recv_bytes = recv(sockfd, recv_buf, MAX_TCP_TRANSMIT_SIZE, 0)) > 0)
        {
            if ((write_bytes = write(fd, recv_buf, recv_bytes)) == -1)
            {
                perror("文件写入失败");
                close(fd);
                return WRITE_FAILED;
            }
            
            printf("客户端收到%ld个字节数\n", recv_bytes);
            
            //每次文件大小减去接收到的字节数，减到0跳出循环
            file_length -= recv_bytes;
            if (file_length == 0)
            {
                printf("下载成功!\n");
                break;
            }
            
            memset(recv_buf, 0, sizeof(recv_buf));
        }
    }
	
	if (recv_bytes < 0)
	{
		perror("接收消息失败");
		return RECV_ERROR;
	}
	return OK;
}

void error_code_show(int err)
{
	switch(err)
	{
		case ERROR:
			printf("程序出错！\n");
			break;
		case CONNECT_ERROR:
			printf("连接出错\n");
			break;
		case RECV_ERROR:
			printf("接收错误\n");
			break;
		case SEND_ERROR:
			printf("发送出错\n");
			break;
		case ACCEPT_ERROR:
			printf("获取套接字出错\n");
			break;
		case LISTEN_ERROR:
			printf("监听出错\n");
			break;
		case BIND_ERROR:
			printf("绑定出错\n");
			break;
		case SOCK_ERROR:
			printf("套接字出错\n");
			break;
		case QUIT:
			printf("客户端主动退出\n");
			break;
		case STR_FORMAT_ERROR:
			printf("字符串格式错误\n");
			break;
		case FUNCID_NOTFOUND:
			printf("业务编号未找到\n");
			break;
		case OPEN_FAILED:
			printf("文件打开出错\n");
			break;
		case CHECK_STAT_FAILED:
			printf("获取文件状态信息失败\n");
			break;
		case WRITE_FAILED:
			printf("文件写入失败\n");
			break;
		default:
			printf("程序出错，错误代码未知\n");
	}
}