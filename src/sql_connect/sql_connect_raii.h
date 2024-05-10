//
// Created by pyq on 5/9/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_SQL_CONNECT_RAII_H
#define SLIM_WEB_SERVER_SQL_CONNECT_RAII_H

#include "sql_connect.h"

// RAII wrapper class for automatic MySQL connection management.
class SqlConnRAII {
public:
    // Constructor acquires a connection from the pool and stores it.
    SqlConnRAII(MYSQL** sqlCoon, SqlConnPool* connPool) {
        assert(connPool);
        *sqlCoon = connPool->GetConn();
        sqlCoon_ = *sqlCoon;
        connPool_ = connPool;
    }

    // Destructor releases the connection back to the pool.
    ~SqlConnRAII() {
        if (sqlCoon_ && connPool_) {
            connPool_->FreeConn(sqlCoon_);
        }
    }
private:
    MYSQL* sqlCoon_;            // Pointer to the MySQL connection.
    SqlConnPool* connPool_;     // Pointer to the connection pool.
};

#endif //SLIM_WEB_SERVER_SQL_CONNECT_RAII_H
