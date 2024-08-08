/**
 * 头文件声明：whackamole游戏功能函数定义
 */
#ifndef __WHACKAMOLE_H__
#define __WHACKAMOLE_H__

#include "../netcore/net.h"

/**
 * 函数声明: 查询回调函数
 * 参数：1. 回调函数参数 2. 列数 3. 列名数组 4. 列值数组
 * 返回：成功返回OK，失败返回代码
 */
static int whackamoleQueryCallback(void *NotUsed, int argc, char **argv, char **azColName);

/**
 * 函数功能：打地鼠游戏入口函数(客户端)
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_start_client(PSESSION pSession, int sockfd, void *args);

/**
 * 函数功能：打地鼠游戏入口函数（服务端）
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_start_server(PSESSION pSession, int sockfd, void *args);

/**
 * 函数功能：打地鼠游戏逻辑处理
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_game_server(PSESSION pSession, int sockfd, void *args);

/**
 * 函数功能：打地鼠游戏开始（服务端）
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_start_game(PSESSION pSession, int sockfd, void *args);

/**
 * 函数功能：打地鼠游戏菜单
 */
void whackamole_menu();
#endif