//
// Created by pyq on 5/9/24.
//

#include "sql_connect.h"

SqlConnPool::SqlConnPool() : useCount_(0), freeCount_(0) {}

SqlConnPool::~SqlConnPool() {
    ClosePool();
}

SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool instance;
    return &instance;
}

void SqlConnPool::Init(const char* host, int port, const char* user, const char* pwd, const char* dbName, int connSize) {
    assert(connSize > 0);
    for (int i = 0; i < connSize; ++i) {
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        connQue_.push(sql);
        freeCount_++;
    }
    MAX_CONN_ = connSize;
    sem_init(&sem_, 0, MAX_CONN_);

}

MYSQL* SqlConnPool::GetConn() {
    sem_wait(&sem_);
    std::lock_guard<std::mutex> locker(mutex_);
    if (connQue_.empty()) {
        return nullptr;
    }
    MYSQL* sqlConn = connQue_.front();
    connQue_.pop();
    freeCount_--;
    useCount_++;
    return sqlConn;
}

void SqlConnPool::FreeConn(MYSQL* sqlConn) {
    assert(sqlConn);
    std::lock_guard<std::mutex> locker(mutex_);
    connQue_.push(sqlConn);
    freeCount_++;
    useCount_--;
    sem_post(&sem_);
}

int SqlConnPool::GetFreeConnCount() {
    std::lock_guard<std::mutex> locker(mutex_);
    return freeCount_;
}

void SqlConnPool::ClosePool() {
    std::lock_guard<std::mutex> locker(mutex_);
    while (!connQue_.empty()) {
        auto sqlConn = connQue_.front();
        connQue_.pop();
        mysql_close(sqlConn);
    }
    useCount_ = 0;
    freeCount_ = 0;
    mysql_library_end();
}