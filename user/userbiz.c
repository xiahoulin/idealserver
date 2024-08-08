#include "user.h"

/**
 * 功能：客户端登录业务处理
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int login_client(PSESSION pSession, int sockfd, void *args)
{
    char **params = (char **) args;
    char *account = params[1];
    char *password = params[2];
    char recv_buf[MAX_TCP_TRANSMIT_SIZE] = {0};

    //接收登录结果
    ssize_t recv_bytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
    if (recv_bytes == -1) 
    {
        perror("Error receiving data");
        return RECV_ERROR;
    }
    
    printf("登录结果：%s\n", recv_buf);
    if (strcmp(recv_buf, "failed") == 0)
    {
        return LOGIN_FAILED;
    }

    return OK;
}

/**
 * 功能：服务端登录业务处理
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int login_server(PSESSION pSession, int sockfd, void *args)
{
    char **params = (char **) args;
    char *account = params[1];
    char *password = params[2];
    char send_buf[MAX_TCP_TRANSMIT_SIZE] = {0};

    //根据账号找出用户信息
    USER user = {0};
    strcpy(user.account, account);
    if (select_user(pSession, &user) != OK)
    {
        strcpy(send_buf, "failed");
        printf("用户不存在\n");
    }
    else
    {
        printf("用户存在\n");
    }

    printf("用户信息：id = %d, account = %s, username = %s\n", user.id, user.account, user.password);
    printf("用户信息传入： account = %s, password = %s\n", account, password);
    if (0 != strcmp(user.password, password))
    {
        strcpy(send_buf, "failed");
        printf("密码错误\n");
    }
    else
    {
        strcpy(send_buf, "ok");
        printf("密码正确\n");
    }

    //开启sesion后保存用户信息到session
    if (pSession != NULL)
    {
        //登录后，保存用户信息到session
        pSession->id = user.id;
        strcpy(pSession->account, user.account);
        strcpy(pSession->username, user.username);
    }
    printf("登录后session信息为： sockfd: %d, clifd = %d, account = %s, userid = %d\n", pSession->sockclifd, pSession->sockclifd, pSession->account, pSession->id);

    //发送登录结果
    ssize_t send_bytes = send(sockfd, send_buf, strlen(send_buf), 0);
    if (send_bytes == -1) 
    {
        perror("Error sending data");
        return SEND_ERROR;
    }

    return OK;
}

/**
 *  功能：客户端注册业务处理
 * 参数：1. session描述符  2.客户端socket文件描述符   3. 处理参数 
 *  返回：成功返回OK，失败返回代码
 */
int register_client(PSESSION pSession, int sockfd, void *args)
{
    char **params = (char **) args;
    char *account = params[1];
    char *password = params[2];
    char recv_buf[MAX_TCP_TRANSMIT_SIZE] = {0};

    //接收注册结果
    ssize_t recv_bytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
    if (recv_bytes == -1) 
    {
        perror("Error receiving data");
        return RECV_ERROR;
    }
    printf("注册结果：%s\n", recv_buf);
    return OK;
}

/**
 * 功能：服务端注册业务处理
 * 参数：1. session描述符  2.客户端socket文件描述符   3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
 int register_server(PSESSION pSession, int sockfd, void *args)
 {
    char **params = (char **) args;
    char *account = params[1];
    char *password = params[2];
    char send_buf[MAX_TCP_TRANSMIT_SIZE] = {0};
    int res = OK;
    //根据账号找出用户信息
    USER user = {0};
    strcpy(user.account, account);
    strcpy(user.password, password);

    if (select_user(pSession, &user) == OK)
    {
        res = USER_EXIST;
        strcpy(send_buf, "该账号已存在");
        printf("该账号已存在\n");
    }
    else
    {
        //插入数据
        if ((res = insert_user(pSession, &user)) < 0)
        {
            strcpy(send_buf, "注册失败,服务器错误");
        }
        strcpy(send_buf, "注册成功");
        printf("注册成功\n");
    }
    //发送登录结果
    ssize_t send_bytes = send(sockfd, send_buf, strlen(send_buf), 0);
    if (send_bytes == -1) 
    {
        perror("Error sending data");
        return SEND_ERROR;
    }
    return OK;
 }

 /**
 * 函数功能：显示用户模块错误
 * 参数：1.错误代码
 */
void show_user_err(int err)
{
    switch (err)
    {
        case SQL_PRE_ERR:
            printf("sql预编译失败\n");
            break;
        case DB_UPDATE_ERR:
            printf("数据库更新失败\n");
            break;
        case DB_DELETE_ERR:
            printf("数据库数据删除失败\n");
            break;
        case DB_SELECT_ERR:
            printf("数据库查看失败\n");
            break;
        case SQL_EXEC_ERR:
            printf("数据库执行错误\n");
            break;
        case DB_INSERT_ERR:
            printf("数据库插入失败\n");
            break;
        case DB_OPEN_ERR:
            printf("数据库打开失败\n");
            break;
        case USER_NOTFOUND:
            printf("用户不存在\n");
            break;
        case USER_ERR:
            printf("数据库错误\n");
            break;
        default:
            printf("未知错误\n");
            break;
    }
}