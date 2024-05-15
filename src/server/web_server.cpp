//
// Created by pyq on 5/13/24.
//
#include "web_server.h"

WebServer::WebServer(
        int port, int trigMode, int timeoutMs, bool optLinger,
        int sqlPort, const char* sqlUser, const char* sqlPwd,
        const char* dbName, int sqlConnPoolNum, int threadNum,
        bool enableLog, int logLevel, int logQueSize) :
        port_(port), openLinger_(optLinger), timeoutMs_(timeoutMs), isClose_(false),   
        timer_(new Timer()), threadPool_(new ThreadPool(threadNum)), epoller_(new Epoller()) {
    // getcwd returns the program's startup directory
    srcDir_ = getcwd(nullptr, 256);    
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16);

    // init log 
    if (enableLog) {
        Log::Instance()->Init(logLevel, "./log", ".log", logQueSize);
    }

    // init http connect static varible
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;

    // init sql connect pool
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, sqlConnPoolNum);

    // init epoll event mode
    InitEventMode_(trigMode);

    // init listen socket
    if (!InitListenSocket_()) {
        isClose_ = true;
    }

    if (enableLog) {
        if (isClose_) { 
            LOG_ERROR("========== Slime-Web-Server Init Error! =========="); 
        }
        else {
            LOG_INFO("========== Server Init ==========");
            LOG_INFO("Port: %d, OpenLinger: %s", port_, optLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent_ & EPOLLET ? "ET": "LT"),
                            (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys Level: %d", logLevel);
            LOG_INFO("SrcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool Capacity: %d, ThreadPool Capacity: %d", sqlConnPoolNum, threadNum);
        }
    }
}
    
WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::Start() {
    // set epoll_wait timeout to -1
    // without events, epoll_wait will block
    int timeMs = -1;
    if (!isClose_) {
        LOG_INFO("========== Server Start ==========");
    }
    while (!isClose_) {
        if (timeoutMs_ > 0) {
            // clear inactive connections 
            // and return the expiration time 
            // of the next earliest expiring connection
            timeMs = timer_->GetNextTick();
        }
        // epoll fd listen events of the http connection fd (listn/connect fd)
        // if no event occurs, it will block for up to timeMs.
        // if the time exceeds, the http connection will be closed.
        int eventCnt = epoller_->Wait(timeMs);
        // handle events listened by epoll 
        for (int i = 0; i < eventCnt; ++i) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if (fd == listenFd_) {
                // new listen connection
                DealListen_();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // an error or connection being suspended, close the corresponding connection.
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            } else if (events & EPOLLIN) {
                // readable event
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            } else if (events & EPOLLOUT) {
                // writeable event
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected Event!");
            }
        }
    }
}

// create listen fd
bool WebServer::InitListenSocket_() {
    int ret;
    sockaddr_in addr;
    if (port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port: %d Error!", port_);
        return false;
    }
    // configure an IPv4 socket, 
    // bound to all available network interfaces, 
    // and listens on a specified port.
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    // control socket closing behavior
    linger optLiner = {0};
    if (openLinger_) {
        // enable linger
        optLiner.l_onoff = 1;
        // set the actual release of the delayed socket to 1s
        optLiner.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        LOG_ERROR("Create Listen Socket Fd Failed!");
        return false;
    }

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLiner, sizeof(optLiner));
    if (ret < 0) {
        LOG_ERROR("Init Linger Error!");
        close(listenFd_);
        return false;
    }

    // bind the socket and address
    ret = bind(listenFd_, (sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERROR("Bind Port: %d Error!", port_);
        close(listenFd_);
        return false;
    }

    // set the server socket (listenFd_) to the listening state, 
    // the maximum number of queued connections allowed is 6
    ret = listen(listenFd_, 6);
    if(ret < 0) {
        LOG_ERROR("Listen Port:%d Error!", port_);
        close(listenFd_);
        return false;
    }

    // add listen fd to epoll's listening queue
    // monitor whether the descriptor is readable, 
    // that is whether there is a new connection
    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if (ret == 0) {
        LOG_ERROR("Add Listen Fd to Epoll's Listening Queue Error!");
        close(listenFd_);
        return false;
    } 

    // set listen fd non block mode
    SetFdNonBlock(listenFd_);
    LOG_INFO("Slim Web Server Port: %d", port_);
    return true;   
}

int WebServer::SetFdNonBlock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void WebServer::InitEventMode_(int trigMode) {
    // Initialize both listening and connection events to detect hang-ups
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLRDHUP | EPOLLONESHOT;

    // Switch based on the provided triggering mode
    switch (trigMode) {
        case 0:
            // Mode 0: Basic level-triggered mode, no changes needed
            break;
        case 1:
            // Mode 1: Connection events are set to edge-triggered
            connEvent_ |= EPOLLET;
            break;
        case 2:
            // Mode 2: Listening events are set to edge-triggered
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            // Mode 3: Both listening and connection events are set to edge-triggered
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
        default:
            // Default case: Treat as Mode 3 for safety, setting both to edge-triggered
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
    }

    // Set the static variable to true if connection events are edge-triggered
    HttpConn::isET = (connEvent_ & EPOLLET);
}

void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].Init(fd, addr);
    if (timeoutMs_ > 0) {
        // use a timer_ to set a timeout callback (Closeconn_) for this client.
        timer_->Add(fd, timeoutMs_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    // add connect fd of this client to epoll's listening queue
    // monitor whether the descriptor is readable, 
    epoller_->AddFd(fd, connEvent_ | EPOLLIN);
    SetFdNonBlock(fd);
    LOG_INFO("Client[%d] In!", users_[fd].GetFd());
}

// handle the listening socket and accept new client connection requests
void WebServer::DealListen_() {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (sockaddr*)&addr, &len);
        if (fd <= 0) {
            // in edge-triggered mode, one event may represent multiple connection requests, 
            // so we need to continue trying to accept until the accept function returns 0 or a negative value, 
            // indicating that there are currently no more connections to accept.
            return;
        } else if (HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "Server Busy!");
            LOG_WARN("Clients is Full!");
            return;
        }
        AddClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

// add a new read task to the server's thread pool
void WebServer::DealRead_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadPool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

// add a new write task to the server's thread pool
void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadPool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

// send error info to the fd 
void WebServer::SendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("Send Error to Client[%d] Error!", fd);
    }
    close(fd);
}

// adjust or update the timeout for a specified client connection
// make sure its connection will not be closed due to timeout
void WebServer::ExtentTime_(HttpConn* client) {
    assert(client);
    if (timeoutMs_ > 0) {
        timer_->Adjust(client->GetFd(), timeoutMs_);
    }
}

// close http connection
// and delete fd in epollfd
void WebServer::CloseConn_(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] Quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::OnRead_(HttpConn* client) {
    assert(client);
    int ret = -1, readErrno = 0;
    ret = client->Read(&readErrno);
    // error
    if (ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess_(client);
}

void WebServer::OnWrite_(HttpConn* client) {
    assert(client);
    int ret = -1, writeErrno = 0;
    ret = client->Write(&writeErrno);
    if (client->ToWriteBytes() == 0) {
        // all data has been writen
        if (client->IsKeepAlive()) {
            OnProcess_(client);
            return;
        }
    } else if (ret < 0) {
        // unable to write more data, but should try again later
        if (writeErrno == EAGAIN) {
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

// Handle http requests and responses
void WebServer::OnProcess_(HttpConn* client) {
    if (client->Process()) {
        // http response is ready to send 
        // so we listen for writable events
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    } else {
        // no http request
        // so we listen for readable events
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}