#include "user.h"

/**
 * 函数声明: 查询回调函数
 * 参数：1. 回调函数参数 2. 列数 3. 列名数组 4. 列值数组
 * 返回：成功返回OK，失败返回代码
 */
static int userQueryCallback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return OK;
}

/**
 * 函数声明: 新增用户
 * 参数：1. session描述符   2. 新增用户信息
 * 返回：成功返回OK，失败返回代码
 */
int insert_user(PSESSION pSession, USER *user)
{
    sqlite3 *db;
    int rc;
    sqlite3_stmt *stmt;

    //打开或创建数据库
    rc = sqlite3_open(DB_PATH, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_OPEN_ERR;
    }

    /* 插入数据 */
    const char *sql_insert = "insert into user (account, username, password) VALUES (?, ?, ?);";

    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);

    if (rc != SQLITE_OK) 
    {
        fprintf(stderr, "SQL prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return SQL_PRE_ERR;
    }

    rc = sqlite3_bind_text(stmt, 1, user->account, -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(stmt, 2, user->username, -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(stmt, 3, user->password, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) 
    {
        fprintf(stderr, "SQL insert failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_INSERT_ERR;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return OK;
}

/**
 * 函数声明：查找用户
 * 参数：1. session描述符 2. 返回用户信息
 * 返回：成功返回OK，失败返回代码
 */
int select_user(PSESSION pSession, USER *user)
{
    sqlite3 *db;
    int rc = 0;
    sqlite3_stmt *stmt;
    //接收用户参数

    //打开或创建数据库
    rc = sqlite3_open(DB_PATH, &db);
    printf("path = %s, rc = %d\n", DB_PATH, rc);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_OPEN_ERR;
    }

    /* 查询数据 */
    const char *sql_select = "SELECT id, account, username, password from user where account = ?;";

    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL);
    if (rc != SQLITE_OK) 
    {
        fprintf(stderr, "SQL prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return SQL_PRE_ERR;
    }

    //绑定查询数据
    rc = sqlite3_bind_text(stmt, 1, user->account, -1, SQLITE_TRANSIENT);

    //查找，每个账号只对应一个用户
    //获取查询结果集数量
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        count++;
        user->id = sqlite3_column_int(stmt, 0);
        strcpy(user->account, sqlite3_column_text(stmt, 1));
        strcpy(user->username, sqlite3_column_text(stmt, 2));
        strcpy(user->password, sqlite3_column_text(stmt, 3));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    //查询结果集数量为0，说明用户不存在
    if (count == 1)
    {
        return OK;
    }else
    {
        return USER_NOTFOUND;
    }
}

/**
 * 函数声明: 用户更新
 * 参数：1. session描述符  2. 更新的用户信息
 * 返回：成功返回OK，失败返回代码
 */
int update_user(PSESSION pSession, USER *user)
{
    sqlite3 *db;
    int rc;
    sqlite3_stmt *stmt;

    //打开或创建数据库
    rc = sqlite3_open(DB_PATH, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_OPEN_ERR;
    }

    /* 更新数据 */
    const char *sql_update = "update user set username = ?, password = ? where id = ?;";

    rc = sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return SQL_PRE_ERR;
    }

    rc = sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(stmt, 2, user->password, -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_int(stmt, 3, user->id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL update failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_UPDATE_ERR;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return OK;
}

/**
 * 函数声明: 用户删除
 * 参数：1. session描述符   2. 处理参数(一个参数，user id)
 * 返回：成功返回OK，失败返回代码
 */
int delete_user(PSESSION pSession, USER *user)
{
    sqlite3 *db;
    int rc;
    sqlite3_stmt *stmt;

    //打开或创建数据库
    rc = sqlite3_open(DB_PATH, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_OPEN_ERR;
    }

    /* 删除数据 */
    const char *sql_delete = "delete from user where id = ?;";

    rc = sqlite3_prepare_v2(db, sql_delete, -1, &stmt, NULL);

    if (rc != SQLITE_OK) 
    {
        fprintf(stderr, "SQL prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return SQL_PRE_ERR;
    }

    rc = sqlite3_bind_int(stmt, 1, user->id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) 
    {
        fprintf(stderr, "SQL delete failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_DELETE_ERR;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return OK;
}

/**
 * 函数声明: 用户查询
 * 参数：1. session描述符   2. 返回的用户列表
 * 返回：成功返回OK，失败返回代码
 */
int query_user(PSESSION pSession, USER **userList)
{   
    sqlite3 *db;
    int rc;
    sqlite3_stmt *stmt;
    //打开或创建数据库
    rc = sqlite3_open(DB_PATH, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_OPEN_ERR;
    }

    /* 查询数据 */
    const char *sql_select = "select id, account, username, password from user;";

    rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL);

    if (rc != SQLITE_OK) 
    {
        fprintf(stderr, "SQL prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return DB_SELECT_ERR;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        int numColumns = sqlite3_column_count(stmt);
        char **columnValues = (char **)malloc(numColumns * sizeof(char *));
        char **columnNames = (char **)malloc(numColumns * sizeof(char *));

        for (int i = 0; i < numColumns; i++) {
            columnNames[i] = (char *)sqlite3_column_name(stmt, i);
            columnValues[i] = (char *)sqlite3_column_text(stmt, i);
        }

        userQueryCallback(NULL, numColumns, columnValues, columnNames);

        free(columnValues);
        free(columnNames);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return OK;
}