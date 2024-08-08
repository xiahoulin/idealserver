#ifndef _NET_H_
#define _NET_H_


//接收转化字符串的最大数量
#define MAX_PARAMS_SIZE 20
//定义文件传输的最大字节数
#define MAX_TCP_TRANSMIT_SIZE 1024
//定义文件传输的最大字节数
#define MAX_UDP_TRANSMIT_SIZE 1024
//单次发消息的最大窗口
#define MAX_MESSAGE_SIZE 100
//最大的EPOLL事件
#define MAX_EPOLL_EVENTS 1024
//广播类型
#define BROADCAST "BROADCAST"
//组播类型
#define MULTICAST "MULTICAST"

#define DB_PATH "./db/testbiz.db"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <strings.h>
#include <netinet/in.h>

//一次连接的session信息
typedef struct session_info
{
	//用户 id
	int id;
	//用户account
	char account[20];
	//用户名
	char username[20];
	//服务端套接字描述符
	int socksrvfd;
	//客户端套接字描述符
	int sockclifd;
	//链表指针
	struct session_info *next;

}SESSION, *PSESSION;

typedef struct sockaddr_in Addr_in;

//TCP返回值代码枚举
enum TCP_RET
{
	//返回错误
	TCP_ERROR = 201,
	//返回成功
	TCP_OK = 200
};

//定义枚举，表示返回值
enum SOCKET_RET
{
	ERR_NOT_FOUND =	-20,
	ERR_MALLOC = -19,
	SET_OPTION_FAILED = -18,
	EPOLL_DEL_ERROR = -17,
	EPOLL_ADD_ERROR = -16,
	EPOLL_CREATE_ERROR = -15,
	WRITE_FAILED = -14,
	CHECK_STAT_FAILED = -13,
	OPEN_FAILED = -12,
	FUNCID_NOTFOUND = -11,
	STR_FORMAT_ERROR = -10,
	QUIT = -9,
	SOCK_ERROR = -8,
	BIND_ERROR,
	LISTEN_ERROR,
    ACCEPT_ERROR,
	SEND_ERROR,
	RECV_ERROR,
	CONNECT_ERROR,
	ERROR,
	OK
};

//角色枚举，客户端还是服务端
enum ROLE
{
	UDPCLIENT = 3,
	UDPSERVER = 2,
	CLIENT = 1,
	SERVER
};

/**
 * 函数功能：初始化socket
 * 参数：1.IP地址  2.端口号
 * 返回：成功返回OK，失败返回代码
 */
int server_initial_func(const char *IP, const char *Port);

/**
 * 函数功能：初始化socket
 * 参数：1.IP地址  2.端口号
 * 返回：成功返回OK，失败返回代码
 * */
int client_initial_func(const char *IP, const char *Port);

/**
 * 函数功能：初始化UDP CLIENT socket
 * 参数：1.IP地址  2.端口号 3.服务端地址
 * 返回：成功返回OK，失败返回代码
 */
int udp_client_initial_func(const char *IP_Server, const char *Port_Server, const Addr_in *serverAddr);

/**
 * 函数功能：初始化UDP SERVER socket
 * 参数：1.IP地址  2.端口号
 * 返回：成功返回OK，失败返回代码
 */
int server_initial_func(const char *IP_Server, const char *Port_Server);

/** 
 *  功能：TCP服务端通信函数
 * 参数：1.socket描述符
 * 返回：成功返回OK，失败返回代码
 **/
int server_com_func(int newfd, PSESSION pSession);

/**
 * 指令发送
 * 参数：1.sockfd 2.order指令 3.pSession会话信息
 * 返回值：成功返回OK，失败返回错误码
 */
int send_order(int sockfd, char *order, PSESSION pSession);

/** 
 * 函数功能：TCP客户端通信函数
 * 参数：1.socket描述符
 * 返回：成功返回OK，失败返回代码
 * */
int client_com_func(int sockfd);

/**
 * 函数功能：TCP服务端启动
 * 参数：1.服务IP  2.服务端口
 * 返回：成功返回OK，失败返回代码
 */
void *tcp_server_bootstrap(void *args);

/**
 * 函数功能：TCP客户端启动
 * 参数：1.服务IP  2.服务端口
 * 返回：成功返回OK，失败返回代码
 **/
void *tcp_client_bootstrap(void *args);

/**
 * 函数功能：TCP服务端线程启动
 * 参数：1.服务IP  2.服务端口
 */
void *tcp_server_t(void *args);

/**
 * 功能：epoll方式启动非阻塞的服务端
 * 入参： 1.服务IP  2.服务端口
 * */
void *tcp_epoll_server_bootstrap(void *args);

/** 
 * 功能：单线程select方式启动服务端
 * 入参： 1.服务IP  2.服务端口
 */
void *tcp_select_server_bootstrap(void *args);

/**
 * 函数功能：UDP客户端启动
 * 参数：1.服务IP  2.服务端口
 */
void *udp_client_bootstrap(void *args);

/**
 * 
 *
 * */
int udp_client_biz_process(const int sockfd, const Addr_in *serverAddr);

/**
 * 函数功能：广播服务端启动
 * 参数：1.服务IP  2.服务端口
 */
void *udp_broadcast_server_bootstrap(void *args);

/**
 * 函数功能：多播服务端启动
 * 参数：1.服务IP  2.服务端口
 */
void *udp_multicast_server_bootstrap(void *args);

/**
 * 函数功能：UDP服务端启动
 * 参数：1.服务IP  2.服务端口
 */
void *udp_server_bootstrap(void *args);

/**
 *
 * */
int udp_server_biz_process(const int sockfd);

/**
 *	功能：文件发送功能
 *	参数：1. socket描述符   2. 处理参数
 *  返回：成功返回OK，失败返回代码
 * */
int download_file_send(PSESSION pSession, int sockfd, void *args);

/**
 *	功能：文件接收功能
 *	参数：1. socket描述符   2. 处理参数
 *  返回：成功返回OK，失败返回代码
 * */
int download_file_receive(PSESSION session, int sockfd, void *args);

/**
 *  功能：根据错误代码显示错误信息
 *  参数：1. 错误代码
 * */
void error_code_show(int err);

/**
 * 函数作用：初始化session
 * 参数：1. session列表指针
 * 返回：成功返回OK，失败返回代码
 */
PSESSION init_session();

/**
 * 函数作用：新建一个session
 * 参数：1. 服务端fd 2.客户端fd 3.用户ID（没有传NULL）4. 用户账号（没有NULL）5. 用户名（没有传NULL）6. 返回的session
 * 返回：成功返回OK，失败返回代码
 */
int create_update_socket_session(int srvfd, int clifd, int userid, char *account, char *username, PSESSION *pSession);

/**
 * 函数作用：添加session到session列表
 * 参数：1. session列表指针 2. session新建的指针
 * 返回：成功返回OK，失败返回代码
 */
int add_session(PSESSION sessions, PSESSION session);

/**
 * 函数作用：从session列表删除session
 * 参数：1. session列表指针 2. 客户端的fd
 * 返回：成功返回OK，失败返回代码
 */
int remove_session(SESSION *session, int clifd);

/**
 * 函数作用：销毁session列表
 * 参数：1. session列表指针
 * 返回：成功返回OK，失败返回代码
 */
int destroy_session(PSESSION *sessions);

/**
 *  指令解析函数
 *  参数1：初始指令 参数2：指令字符串指针数组 参数3：分割符
 * */
int message_convert(char *origin_str, char **res_str_list, char reg);

/**
 * 函数作用：查找session
 * 参数：1. session列表指针 2. 客户端的fd
 * 返回：成功返回session指针，失败返回NULL
 */
PSESSION find_session(PSESSION sessions, int clifd);


/** 功能：选择方法路由
 *  参数1：功能号ID，参数2：功能参数 参数3：客户端套接字描述符，参数4：角色 参数5：会话
 *  返回：成功返回OK，失败返回代码
 */
int cmd_func_router(char *funcID, char *args[], int sockfd, int role, PSESSION pSession);

#endif
