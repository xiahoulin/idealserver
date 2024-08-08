#include "whackamole.h"

int moleloc;
void flush_input_buffer(void) {
    int c;
    // 循环读取字符直到换行符或EOF
    while ((c = getchar()) != '\n' && c != EOF) {
        // 什么都不做，只是读取并丢弃字符
    }
}
/**
 * 函数功能：打地鼠游戏入口函数
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_start_client(PSESSION pSession, int sockfd, void *args)
{
    int res = 0;
    //初始化客户端 session
    if ((res = create_update_socket_session(0, sockfd, 0, NULL, NULL, &pSession)) < 0 )
    {
        printf("create_update_socket_session failed!\n");
        return res;
    }

    //账号，用户名，密码
    char account[20] = {0};
    char username[20] = {0};
    char password[20] = {0};
    //接收数据缓冲区
    char send_buf[MAX_TCP_TRANSMIT_SIZE] = {0};
    char recv_buf[MAX_TCP_TRANSMIT_SIZE] = {0};

    //函数功能
    int func = 0;
    while(1)
    {
        whackamole_menu();
        scanf("%d", &func);
        if (5 == func)
        {
            break;
        }
        else if (1 == func)
        {
            //开始游戏指令
            char order[MAX_MESSAGE_SIZE] = {0};;
            //开始游戏
            if (0 == strlen(account))
            {
                printf("请先登录！\n");
                continue;
            }

            sprintf(order, "#whackamole#%s", account);
            send_order(sockfd, order, pSession);
            
            //发送开始游戏指令
            while(1)
            {
                //接收游戏开始指令
                memset(recv_buf, 0, sizeof(recv_buf));
                ssize_t recv_bytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
                printf("recved: %s\n", recv_buf);
                if (recv_bytes <= 0)
                {
                    perror("Error receiving data");
                    return RECV_ERROR;
                }

                if (0 == strncmp(recv_buf, "wait", 4))
                {
                    printf("等待下一轮开始！\n");
                    continue;
                }
                else if (0 == strncmp(recv_buf, "join", 4))
                {
                    printf("成功加入游戏！\n");
                    continue;
                }
                else if (0 == strncmp(recv_buf, "hit", 3))
                {
                    printf("真棒！等待下一轮开始\n");
                    continue;
                }
                else if (0 == strncmp(recv_buf, "ok", 2))
                {
                    printf("游戏开始！\n");
                }
                else if (0 == strncmp(recv_buf, "exit", 4))
                {
                    printf("游戏结束！\n");
                    break;
                }
                else if (0 == strncmp(recv_buf, "nothit", 6))
                {
                    printf("很遗憾，没有击中！\n");
                }
                

                memset(send_buf, 0, sizeof(send_buf));
                printf("请输入老鼠的位置：\n");
                scanf("%s", send_buf);

                //发送位置
                memset(order, 0, sizeof(order));
                sprintf(order, "#hitmole#%s", send_buf);
                if ((res = send_order(sockfd, order, pSession)) < 0)
                {
                    printf("send_order failed!\n");
                    return res;
                }

                
            }
        }
        else if (2 == func)
        {
            //登录账号
            char order[MAX_MESSAGE_SIZE] = {0};

            //开始游戏
            if (0 != strlen(account))
            {
                printf("用户已登录！\n");
                continue;
            }

            printf("请输入账号：\n");
            scanf("%s", account);
            printf("请输入密码：\n");
            scanf("%s", password);

            sprintf(order, "#login#%s#%s", account, password);
            //发送登录指令
            if ((res = send_order(sockfd, order, pSession)) < 0)
            {
                memset(account, 0, sizeof(account));
                memset(password, 0, sizeof(password));
                continue;
            }
            //信息放入session
            strcpy(pSession->account, account);
        }
        else if (3 == func)
        {
            //注册账号
            char order[MAX_MESSAGE_SIZE] = {0};
            char check_password[20] = {0};

            //判断是否登录
            if (0 != strlen(account))
            {
                printf("用户已登录！\n");
                continue;
            }

            //密码一致性校验
            int flag = 1;
            while(flag != 0)
            {
                printf("请输入账号：\n");
                scanf("%s", account);
                printf("请输入密码：\n");
                scanf("%s", password);
                printf("请再次输入新密码：\n");
                scanf("%s", check_password);
                flag = strcmp(password, check_password);
            }

            sprintf(order, "#register#%s#%s", account, password);
            send_order(sockfd, order, pSession);
            memset(account, 0, sizeof(account));
            memset(password, 0, sizeof(password));
        }
        else if (4 == func)
        {
            //查看记录
        }
    }
    return OK;
}

/**
 * 函数功能：打地鼠游戏入口函数（服务端）
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_start_server(PSESSION pSession, int sockfd, void *args)
{
    PSESSION cliSession = find_session(pSession, sockfd);
    //通知客户端游戏行为
    ssize_t send_bytes = send(sockfd, "join", 4, 0);
    printf("玩家：%s 加入游戏！\n", cliSession->account);
    if (send_bytes <= 0)
    {
        perror("Error sending data");
        return SEND_ERROR;
    }

    return OK;
}

/**
 * 函数功能：打地鼠游戏开始（服务端）
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_start_game(PSESSION pSession, int sockfd, void *args)
{
    //游戏前端传入的参数
    char **params = (char **) args;
    char *game_cmd = params[1];
    //数据缓冲区
    char send_buf[MAX_TCP_TRANSMIT_SIZE] = {0};
    //随机生成1-9的数字，作为地鼠出现的位置
    moleloc = rand() % 9 + 1;
    //游戏客户端遍历指针
    PSESSION pGame = pSession->next;

    //通知客户端游戏开始
    while(pGame != NULL)
    {
        //不是游戏展示客户端
        if (sockfd == pGame->sockclifd)
        {
            continue;
        }

        //
        ssize_t send_bytes = 0;
        printf("cmd = %s\n", game_cmd);
        if (0 == strncmp(game_cmd, "start", 5))
        {
            printf("游戏开始，当前地鼠出现在%d号位置\n", moleloc);
            send_bytes= send(pGame->sockclifd, "ok", 2, 0);
        }
        else 
        {
            send_bytes= send(pGame->sockclifd, "exit", 4, 0);
        }
        

        if (send_bytes <= 0)
        {
            perror("Error sending data");
            return SEND_ERROR;
        }
        pGame = pGame->next;
    }

    return OK;
}



/**
 * 函数功能：打地鼠游戏逻辑处理
 * 参数：1. session描述符   2. 客户端socket文件描述符  3. 处理参数
 * 返回：成功返回OK，失败返回代码
 */
int whackamole_game_server(PSESSION pSession, int sockfd, void *args)
{
    ssize_t send_bytes = 0;
    char **params = (char **) args;
    int inputloc = atoi(params[1]);
    printf("in hit*******\n");

    //胜负逻辑
    if (moleloc == 0)
    {
        //如果地鼠位置被修改通知等待
        send_bytes = send(pSession->sockclifd, "wait", 4, 0);
    }
    else if (moleloc == inputloc)
    {
        //如果地鼠位置未被修改则选中
        moleloc = 0;
        send_bytes = send(pSession->sockclifd, "hit", 3, 0);
    }
    else
    {
        send_bytes = send(pSession->sockclifd, "nothit", 6, 0);
    }

    if (send_bytes <= 0)
    {
        perror("Error sending data");
        return SEND_ERROR;
    }
}

/**
 * 函数功能：打地鼠游戏菜单
 */
void whackamole_menu()
{
    //函数功能
    puts("---------------------------------------------------------------------------------");
    puts("                                  打地鼠online");
    puts("");
    puts("                                  1. 开始游戏");
    puts("                                  2. 登录账号");
    puts("                                  3. 注册账号");
    puts("                                  4. 查看记录");
    puts("                                  5. 退出游戏");
    puts("---------------------------------------------------------------------------------");
}