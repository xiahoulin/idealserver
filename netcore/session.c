#include "net.h"

/**
 * 函数作用：初始化session
 * 参数：1. session列表指针
 * 返回：成功返回OK，失败返回代码
 */
PSESSION init_session()
{
    //分配内存
    PSESSION session = (SESSION *)malloc(sizeof(SESSION));
    if (session == NULL) {
        return NULL;
    }
    memset(session, 0, sizeof(SESSION));

    return session;
}


/**
 * 函数作用：新建一个session
 * 参数：1. 服务端fd 2.客户端fd 3.用户ID（没有传NULL）4. 用户账号（没有NULL）5. 用户名（没有传NULL）6. 返回的session
 * 返回：成功返回OK，失败返回代码
 */
int create_update_socket_session(int srvfd, int clifd, int userid, char *account, char *username, PSESSION *pSession)
{
    if (*pSession == NULL) {
        //分配内存
        *pSession = (SESSION *)malloc(sizeof(SESSION));
        if (*pSession == NULL) {
            return ERR_MALLOC;
        }
    }
    memset(*pSession, 0, sizeof(SESSION));
    //初始化
    (*pSession)->id = userid;
    strcpy((*pSession)->username, (username == NULL) ? "" : username);
    strcpy((*pSession)->account, (account == NULL) ? "" : account);
    (*pSession)->sockclifd = clifd;
    (*pSession)->socksrvfd = srvfd;
    (*pSession)->next = NULL;

    return OK;
}

/**
 * 函数作用：查找session
 * 参数：1. session列表指针 2. 客户端的fd
 * 返回：成功返回session指针，失败返回NULL
 */
PSESSION find_session(PSESSION sessions, int clifd)
{
    PSESSION pSession = sessions;  // 用于遍历链表
    while (pSession != NULL) {
        // 检查是否找到了匹配的sockclifd
        if (pSession->sockclifd == clifd) {
            return pSession;
        }
        // 移动到下一个节点
        pSession = pSession->next;
    }
    return NULL;
}

/**
 * 函数作用：添加session到session列表
 * 参数：1. session列表指针 2. session新建的指针
 * 返回：成功返回OK，失败返回代码
 */
int add_session(PSESSION sessions, PSESSION session)
{
    //前插添加到链表
    session->next = sessions->next;
    sessions->next = session;
    return OK;
}

/**
 * 函数作用：从session列表删除session
 * 参数：1. session列表指针 2. 客户端的fd
 * 返回：成功返回OK，失败返回代码
 */
int remove_session(SESSION *session, int clifd)
{
    PSESSION pSession = session;
    PSESSION pPrev = NULL;  // 用于追踪前一个节点

    // 遍历链表
    while (pSession != NULL) {
        // 检查是否找到了匹配的sockclifd
        if (pSession->sockclifd == clifd) {
            // 如果找到了，更新前一个节点的next指针以跳过当前节点
            if (pPrev != NULL) {
                pPrev->next = pSession->next;
            } else {
                // 如果当前节点是头节点
                session = pSession->next;
            }
            // 释放当前节点的内存
            free(pSession);
            return OK;  // 成功删除
        }
        // 移动到下一个节点
        pPrev = pSession;
        pSession = pSession->next;
    }

    return ERR_NOT_FOUND;
}

/**
 * 函数作用：销毁session列表
 * 参数：1. session列表指针
 * 返回：成功返回OK，失败返回代码
 */
int destroy_session(PSESSION *sessions)
{
    PSESSION pSession = *sessions;  
    while (pSession != NULL) {
        PSESSION nextSession = pSession->next;  // 保存下一个节点的地址
        free(pSession);  // 释放当前节点的内存
        pSession = nextSession;  // 移动到下一个节点
    }
    // 释放头节点
    *sessions = NULL;
    return OK;
}