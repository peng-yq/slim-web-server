//
// Created by peng yongqiang on 2024/5/5.
//
#pragma once
#ifndef SLIM_WEB_SERVER_LOG_H
#define SLIM_WEB_SERVER_LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <sys/stat.h>
#include <cstdarg>
#include <cassert>
#include <cstring>
#include <ctime>
#include "../buffer/buffer.h"
#include "../block_deque/block_deque.h"

// A thread-safe logging class that supports both synchronous and asynchronous logging.
class Log {
public:
    // Retrieves the singleton instance of the Log class.
    static Log* Instance();

    // Triggers asynchronous log flushing.
    static void AsyncFlushLog();

    // Flushes the log buffer to the file.
    void Flush();

    // Writes a formatted log message at a given log level.
    void Write(int level, const char* format, ...);

    // Initializes the logging system with specific parameters.
    void Init(int level = 1, const char* path = "./log", const char* suffix = ".log", int capacity = 1024);

    // Returns the current log level.
    int GetLevel();

    // Sets the current log level.
    void SetLevel(int level);

    // Checks if the log file is open.
    bool IsOpen();

private:
    static const int LOG_PATH_LEN = 256;  // Maximum length of the log path.
    static const int LOG_NAME_LEN = 256;  // Maximum length of the log file name.
    static const int MAX_LINES = 50000;   // Maximum number of lines per log file.

    const char* path_;           // Directory path for log files.
    const char* suffix_;         // Suffix for log files.
    int maxLines_;               // Maximum number of lines allowed in the log file.
    int lineCount_;              // Current count of lines written to the log file.
    int day_;                    // Current day (used for file rotation).
    int level_;                  // Current log level.
    bool isOpen_;                // Flag indicating if the log system is initialized and open.
    bool isAsync_;               // Flag indicating if logging should be asynchronous.
    FILE* fp_;                   // File pointer for the log file.
    Buffer buffer_;              // Buffer for storing log messages before writing to the file.
    std::unique_ptr<BlockDeque<std::string>> blockDeque_; // Queue for asynchronous logging.
    std::unique_ptr<std::thread> writeThread_;           // Thread handling asynchronous log writes.
    std::mutex mutex_;           // Mutex for synchronizing access to the log.

    // Constructor.
    Log();

    // Destructor.
    virtual ~Log();

    // Appends the log level title to the log message.
    void AppendLogLevelTitle(int level);

    // Writes log messages stored in the block deque to the file.
    void AsyncWrite();
};

#define LOG_BASE(level, format, ...) \
    do { \
        Log* log = Log::Instance(); \
        if (log->IsOpen() && log->GetLevel() <= level) { \
            log->Write(level, format, ##__VA_ARGS__); \
            log->Flush();\
        } \
    } while (0)

#define LOG_DEBUG(format, ...) LOG_BASE(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG_BASE(1, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG_BASE(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_BASE(3, format, ##__VA_ARGS__)

#endif // SLIM_WEB_SERVER_LOG_H
