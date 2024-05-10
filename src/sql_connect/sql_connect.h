//
// Created by pyq on 5/9/24.
//
#pragma once
#ifndef SLIM_WEB_SERVER_SQL_CONNECT_H
#define SLIM_WEB_SERVER_SQL_CONNECT_H

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <semaphore.h>
#include <mysql/mysql.h>
#include "../log/log.h"

// SQL connection pool class for managing MySQL connections.
class SqlConnPool {
public:
    // Singleton instance access method.
    static SqlConnPool* Instance();

    // Initializes the connection pool with database parameters and the maximum number of connections.
    void Init(const char* host, int port, 
              const char* user, const char* pwd, 
              const char* dbName, int connSize = 10);
    
    // Retrieves a connection from the pool, blocking if none are available.
    MYSQL* GetConn();

    // Returns a connection to the pool.
    void FreeConn(MYSQL* sqlConn);

    // Returns the number of free connections currently available in the pool.
    int GetFreeConnCount();

    // Closes all connections and destroys the pool.
    void ClosePool();

private:
    int MAX_CONN_;                  // Maximum number of connections allowed in the pool.
    int useCount_;                  // Current count of connections in use.
    int freeCount_;                 // Current count of free connections available.
    std::queue<MYSQL*> connQue_;    // Queue storing available connections.
    std::mutex mutex_;              // Mutex for synchronizing access to the connection queue.
    sem_t sem_;                     // Semaphore for managing available connections.

    // Private constructor for singleton pattern.
    SqlConnPool();  

    // Destructor cleans up connections.
    ~SqlConnPool();

    // Deleted copy constructor.
    SqlConnPool(const SqlConnPool& other) = delete;

    // Deleted assignment operator.
    SqlConnPool& operator=(const SqlConnPool& other) = delete;
};

#endif //SLIM_WEB_SERVER_SQL_CONNECT_H
