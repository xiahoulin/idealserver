#include <stdio.h>
#include <sqlite3.h>

#define DB_PATH "../db/testbiz.db"

int main() {
    sqlite3 *db;
    int rc;
    sqlite3_stmt *stmt;

    /* 打开或创建数据库 */
    rc = sqlite3_open(DB_PATH, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* 创建表 */
    const char *sql_create = "create table if not exists user " 
                            "(id            integer primary key  autoincrement  not null, " 
                            "account        char(20)    not null, " 
                            "username       char(20)    not null, " 
                            "password       char(20));";

    rc = sqlite3_exec(db, sql_create, NULL, 0, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    } else {
        fprintf(stderr, "Table created successfully\n");
    }

    

    sqlite3_close(db);
    return(0);
}