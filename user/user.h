#include "../netcore/net.h"
#include <sqlite3.h>

//用户结构体
typedef struct userinfo
{
    int id;
    char account[20];
    char username[20];
    char password[20];
}USER;

//用户模块错误代码
enum USER_ERR
{
    LOGIN_FAILED = -11,
    USER_EXIST = -10,
    SQL_PRE_ERR = -9,
    DB_DELETE_ERR = -8,
    DB_UPDATE_ERR = -7,
    DB_SELECT_ERR = -6,
    SQL_EXEC_ERR = -5,
    DB_INSERT_ERR = -4,
    DB_OPEN_ERR = -3,
    USER_NOTFOUND = -2,
    USER_ERR = -1
};

/**
 * 函数声明: 新增用户
 * 参数：1. session描述符   2. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int insert_user(PSESSION pSession, USER *user);

/**
 * 函数声明：查找用户
 * 参数：1. session描述符 2. 返回用户信息
 * 返回：成功返回OK，失败返回代码
 */
int select_user(PSESSION pSession, USER *user);

/**
 * 函数声明: 用户更新
 * 参数：1. session描述符  2. 更新的用户信息
 * 返回：成功返回OK，失败返回代码
 */
int update_user(PSESSION pSession, USER *user);

/**
 * 函数声明: 用户删除
 * 参数：1. session描述符   2. 处理参数(一个参数，user id)
 * 返回：成功返回OK，失败返回代码
 */
int delete_user(PSESSION pSession, USER *user);

/**
 * 函数声明: 用户查询
 * 参数：1. session描述符   2. 返回的用户列表
 * 返回：成功返回OK，失败返回代码
 */
int query_user(PSESSION pSession, USER **userList);

/**
 * 功能：客户端登录业务处理
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int login_client(PSESSION pSession, int sockfd, void *args);

/**
 * 功能：服务端登录业务处理
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int login_server(PSESSION pSession, int sockfd, void *args);

/**
 *  功能：客户端注册业务处理
 * 参数：1. session描述符  2.客户端socket文件描述符   3. 处理参数 
 *  返回：成功返回OK，失败返回代码
 */
int register_client(PSESSION pSession, int sockfd, void *args);

/**
 * 功能：服务端注册业务处理
 * 参数：1. session描述符  2.客户端socket文件描述符   3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
 int register_server(PSESSION pSession, int sockfd, void *args);

/**
 * 函数功能：显示用户模块错误
 * 参数：1.错误代码
 */
void show_user_err(int err);

/**
 * 函数声明: 查询回调函数
 * 参数：1. 回调函数参数 2. 列数 3. 列名数组 4. 列值数组
 * 返回：成功返回OK，失败返回代码
 */
static int userQueryCallback(void *NotUsed, int argc, char **argv, char **azColName);