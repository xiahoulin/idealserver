/**
 * 头文件声明：公共头文件
 * 功能：声明方法路由等
 */
#ifndef __COMMON_H__
#define __COMMON_H__


//添加所需的头文件以及函数声明 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//添加所需业务路由服务的头文件以及函数声明 
#include "../whackamole/whackamole.h"
#include "../user/user.h"

//业务函数指针
typedef int (*biz_process)(PSESSION pSession, int sockfd, void *args);

/**
 *  指令解析函数
 *  参数1：初始指令 参数2：指令字符串指针数组 参数3：分割符
 * */
int message_convert(char *origin_str, char **res_str_list, char reg);


/** 功能：选择方法路由
 *  参数1：功能号ID，参数2：功能参数 参数3：客户端套接字描述符，参数4：角色 参数5：会话
 *  返回：成功返回OK，失败返回代码
 */
int cmd_func_router(char *funcID, char *args[], int sockfd, int role, PSESSION pSession);

#endif 