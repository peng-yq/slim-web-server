//
// Created by pyq on 2024/5/6.
//

#include "log.h"

// Constructor: Initializes the log system defaults.
Log::Log() : lineCount_(0), isAsync_(false), day_(0), fp_(nullptr) {}

// Destructor: Ensures all resources are properly released and threads joined.
Log::~Log() {
    if (writeThread_ && writeThread_->joinable()) {
        while (!blockDeque_->empty()) {
            blockDeque_->flush();
        }
        blockDeque_->close();
        writeThread_->join();
    }
    if (fp_) {
        std::lock_guard<std::mutex> locker(mutex_);
        Flush();
        fclose(fp_);
        fp_ = nullptr;
    }
}

// Retrieves the singleton instance of the Log class.
Log* Log::Instance() {
    static Log instance;
    return &instance;
}

// Triggers asynchronous log flushing.
void Log::AsyncFlushLog() {
    Log* log = Log::Instance();
    log->AsyncWrite();
}

// Flushes the log buffer to the file.
void Log::Flush() {
    std::lock_guard<std::mutex> locker(mutex_);
    fflush(fp_);
}

// Writes a formatted log message at a given log level.
void Log::Write(int level, const char* format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    // One log file per day, and the maximum number of lines in a single log file is guaranteed to be MAX_LINES
    if (day_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(mutex_);
        locker.unlock();
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        if (day_ != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            day_ = t.tm_mday;
            lineCount_ = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES), suffix_);
        }
        locker.lock();
        Flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }
    {
        std::unique_lock<std::mutex> locker(mutex_);
        int n = snprintf(buffer_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buffer_.AdvanceWritePointer(n);
        AppendLogLevelTitle(level);
        ++lineCount_;
        va_start(vaList, format);
        int m = vsnprintf(buffer_.BeginWrite(), buffer_.GetWritableBytes(), format, vaList);
        va_end(vaList);
        buffer_.AdvanceWritePointer(m);
        buffer_.Append("\n\0", 2);

        // synchronous or asynchronous logging
        if (isAsync_ && blockDeque_ && !blockDeque_->full()) {
            blockDeque_->push_back(buffer_.RetrieveAllAsString());
        } else {
            fputs(buffer_.BeginRead(), fp_);
        }
        buffer_.RetrieveAll();
    }
}

// Initializes the logging system with specific parameters.
void Log::Init(int level, const char* path, const char* suffix, int capacity) {
    level_ = level;
    isOpen_ = true;
    path_ = strdup(path);
    suffix_ = strdup(suffix);
    if (capacity > 0) {
        isAsync_ = true;
        if (!blockDeque_) {
            std::unique_ptr<BlockDeque<std::string>> blockDeque(new BlockDeque<std::string>(capacity));
            blockDeque_ = move(blockDeque);
            std::unique_ptr<std::thread> writeThread(new std::thread(AsyncFlushLog));
            writeThread_ = move(writeThread);
        }
    } else {
        isAsync_ = false;
    }

    lineCount_ = 0;
    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    day_ = t.tm_mday;

    {
        std::unique_lock<std::mutex> locker(mutex_);
        buffer_.RetrieveAll();
        if (fp_) {
            Flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

// Returns the current log level.
int Log::GetLevel() {
    std::lock_guard<std::mutex> locker(mutex_);
    return level_;
}

// Sets the current log level.
void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> locker(mutex_);
    level_ = level;
}

// Checks if the log file is open.
bool Log::IsOpen() {
    std::lock_guard<std::mutex> locker(mutex_);
    return isOpen_;
}

// Appends the log level title to the log message.
void Log::AppendLogLevelTitle(int level) {
    const char* titles[] = {"[debug]: ", "[info] : ", "[warn] : ", "[error]: "};
    buffer_.Append(titles[level], strlen(titles[level]));
}

// Writes log messages stored in the block deque to the file.
void Log::AsyncWrite() {
    std::string str;
    while (blockDeque_->pop_front(str)) {
        fputs(str.c_str(), fp_);
    }
}
