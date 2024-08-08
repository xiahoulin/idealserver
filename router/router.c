#include "router.h"

/**
 * 消息转换，将字符串转换为字符数组
 * 参数：1.原始字符串 2.字符数组指针 3.分隔符
 */
int message_convert(char *origin_str, char **res_str_list, char reg)
{
	char *p_index = NULL;
	int count = 0, size = 0;
	int length = strlen(origin_str);
	
	//不以reg头尾均为格式错误
	if (reg != origin_str[count])
	{
		return STR_FORMAT_ERROR;
	}
	
	//遍历指针付给开头
	p_index = origin_str + 1;
	res_str_list[count] = p_index;
	while('\0' != *p_index)
	{
		//不等于分隔符
		if (reg != *p_index)
		{
			p_index++;
		}
		else
		{
			//等于分隔符替换尾'\0'并记录当前第一个指针
			*p_index = '\0';
			count ++;
			res_str_list[count] = ++p_index;
		}
	}
	
	//遍历解析结果
	count = 0;
	while(NULL != (p_index = res_str_list[count++]))
	{
		printf("%s ", p_index);
	}
	printf("\n");
	
	return OK;
}

/**
 * 命令行路由，根据命令行参数执行相应的业务
 * 参数：1.方法路由 2.参数列表 3.socketfd 4.角色
 * 返回值：成功OK，失败返回错误码
 */
int cmd_func_router(char *funcID, char *args[], int sockfd, int role, PSESSION pSession)
{
	char **p_args = args;
	biz_process p_biz;
	int biz_res = 0;

	if (0 == strlen(funcID))
	{
		printf("功能号为空！\n");
		return ERROR;
	}

	//开启功能路由
	if (0 == strncasecmp("download", funcID, 8))
	{
		//对于客户端下载调用接收文件
		if (CLIENT == role)
		{
			p_biz = download_file_receive;
		}
		else
		{
			//服务端下载调用发送文件
			p_biz = download_file_send;
		}
	}
	else if (0 == strncasecmp("upload", funcID, 6))
	{
		//对于客户端上传调用发送文件
		if (CLIENT == role)
		{
			p_biz = download_file_send;
		}
		else
		{
			//对于服务端上传调用接收文件
			p_biz = download_file_receive;
		}
	}
    else if (0 == strncasecmp("login", funcID, 5))
    {
        //登录功能，根据服务客户端判断处理函数
        if (CLIENT == role)
        {
            p_biz = login_client;
        }
        else
        {
            p_biz = login_server;
        }
    }
	 else if (0 == strncasecmp("register", funcID, 7))
    {
        //登录功能，根据服务客户端判断处理函数
        if (CLIENT == role)
        {
            p_biz = register_client;
        }
        else
        {
            p_biz = register_server;
        }
    }
	else if (0 == strncasecmp("whackamole", funcID, 10))
	{
        if (SERVER == role)
        {
            p_biz = whackamole_start_server;
        }
		else{
			return OK;
		}
	}
	else if (0 == strncasecmp("game", funcID, 10))
	{
        if (CLIENT == role)
        {
            p_biz = whackamole_start_client;
        }
		else
		{
			return OK;
		}
	}
	else if (0 == strncasecmp("hitmole", funcID, 7))
	{
		//开始打地鼠的游戏
		if (SERVER == role)
        {
            p_biz = whackamole_game_server;
        }
		else
		{
			return OK;
		}
	}
	else if (0 == strncasecmp("startgame", funcID, 9))
	{
		//开始打地鼠的游戏
		if (SERVER == role)
        {
            p_biz = whackamole_start_game;
        }
		else
		{
			return OK;
		}
	}
	else
	{
		return FUNCID_NOTFOUND;
	}

	//业务执行
	if ((biz_res = p_biz(pSession, sockfd, (void *)p_args)) < 0)
	{
		printf("业务执行出错!\n");
		return biz_res;
	}
	return OK;
}

