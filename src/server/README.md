## web_server

### 初始化

1. 初始化web server配置：
   - 端口号 (port_)
   - 触发模式 (trigMode)
   - 连接超时时间 (timeoutMs_)
   - 是否启用监听套接字的LINGER选项 (openLinger_)
   - 数据库相关设置
   - 定时器（timer_）
   - Epoll
   - 线程池大小 (threadNum)
   - 日志系统设置
   - 资源目录（srcDir_）
2. 初始化日志系统，设置日志级别、日志文件存放路径和队列大小。
3. 初始化数据库连接池，配置包括数据库服务器地址、端口、用户名、密码、数据库名以及连接池大小。
4. 根据传入的触发模式 (trigMode) 设置epoll的事件监听模式，决定监听事件和连接事件是使用边缘触发还是水平触发。
5. 调用InitListenSocket_() 方法初始化监听套接字。如果初始化失败，设置isClose_ 标志为true，表示服务器初始化失败。初始化监听套接字会根据openLinger_决定是否开启LINGER选项，初始化成功后会将监听套接字fd加入epoll进行监听。
6. 如果启用了日志，根据isClose_的状态记录不同的日志信息。如果服务器初始化成功，记录服务器的配置信息，如端口、是否启用SO_LINGER、监听模式、日志级别、资源目录、数据库连接池容量和线程池容量等。

### 启动

1. 设置epoll_wait超时：
   - timeMs初始化为-1，意味着如果没有任何事件，epoll_wait将会阻塞，直到有事件发生。
2. 检查服务器是否关闭：
    - 如果isClose_标志为true，则不进入主循环，服务器不会启动。
    - 如果服务器正常启动，记录启动日志。
3. 主事件循环：
    - 循环继续执行，直到isClose_被设置为 true。
    - 如果设置了超时时间 (timeoutMs_ > 0)，通过timer_对象调用GetNextTick()方法计算下一个超时事件的时间。这个时间用于epoll_wait的超时参数，以便在没有网络事件时处理超时连接。GetNextTick()方法会清理超时的连接。
4. 调用epoll_wait监听事件：
   - 使用epoller_->Wait(timeMs)监听文件描述符上的事件，timeMs可能是超时时间或-1（永不超时）。-1对应定时器中没有添加的http连接，即epoll监听监听套接字是否有可读事件，只要无事件发生（无连接）就一直阻塞。返回的是发生事件数量eventCnt。
5. 处理每个事件：
   - 遍历每个事件，使用epoller_->GetEventFd(i)和epoller_->GetEvents(i)获取事件相关的文件描述符fd和事件类型events。
6. 根据不同的情况处理事件：
    - 监听套接字事件：如果fd是监听文件描述符listenFd_，调用DealListen_()方法接受新的连接。DealListen_()方法会将连接的套接字描述符加入定时器和epoll进行监听（读事件，也就是监听httprequest）。
    - 错误或挂起的连接：如果事件类型包含EPOLLRDHUP、EPOLLHUP或EPOLLERR，表示连接出现错误或挂起，调用CloseConn_()方法关闭连接，并从epoll中删除。
    - 可读事件：如果事件类型包含EPOLLIN，表示数据可读，调用DealRead_()方法处理读取数据。DealRead_()方法首先会使HttpConn去读套接字中的内容至读缓冲区，并调用OnProcess_方法处理http事件，该方法首先会调用HttpConn->Process()方法去处理http请求，并构造对应的http响应至写缓冲区。此外，Process方法会根据返回修改该连接套接字的监听事件类：返回true则监听可写事件，用于将http响应进行发送；返回false表示当前没有http请求，需要监听可读事件。
    - 可写事件：如果事件类型包含EPOLLOUT，表示连接可写，调用DealWrite_()方法处理数据发送。DealWrite_()方法首先会让HttpConn去往套接字写入写缓冲区的内容，也就是构造的http响应，如果全部写完则根据http请求是否需要保持连接（keep-alive）决定是否需要继续处理http连接，否则直接关闭。如果没有完全写完，则继续监听可写事件。
    - 异常处理：如果事件不是以上任何一种已知类型，记录错误日志

> 处理可读可写事件均是将任务推送至线程池任务队列，由工作线程来取出完成

### 单reactor多线程

1. 单个Reactor（主线程）：单个Reactor运行在主线程（WebServer.Start()）中，负责监听所有的I/O事件，包括新的客户端连接请求以及现有连接上的读写事件。Reactor使用非阻塞I/O和事件通知机制（epoll），能够高效地处理成千上万的并发连接。

2. 事件分发：当事件发生时，Reactor会从epoll事件队列中获取事件，并根据事件类型（新连接、数据读取、数据写入等）将事件分派给相应的处理程序（ DealListen_, DealRead_或DealWrite_方法。）。

3. 多线程处理：对于读写事件，Reactor不直接处理具体的数据读取或写入逻辑。相反，它将这些任务委托给后端的线程池。这样可以快速地返回到事件循环中，继续监听其他I/O事件，而具体的I/O处理则由工作线程并行处理。

4. 异步执行：工作线程处理完任务后，需要再次通知Reactor线程（主线程）以进行进一步的操作，如发送响应到客户端。这里是通过修改监听对应套接字描述符的事件状态或再次注册事件来实现。

### epoll

```c++
typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;    /* Epoll events */
    epoll_data_t data;      /* User data variable */
};
```

events：这个字段是一个位掩码，指定了需要监听的事件类型。常见的事件类型包括：

- EPOLLIN：表示对应的文件描述符可以读取数据（如有新的连接请求或者有数据可读）。
- EPOLLOUT：表示对应的文件描述符可以写入数据（如写缓冲区有空间可写）。
- EPOLLET：将 epoll 行为设置为边缘触发（Edge Triggered）模式，这是与水平触发（Level Triggered）模式相对的一种高效模式。
- EPOLLERR：表示对应的文件描述符发生了错误。
- EPOLLHUP：表示对应的文件描述符被挂断。
- EPOLLRDHUP：表示套接字的远端关闭或半关闭连接。
- EPOLLONESHOT：表示一次性监听，事件发生一次后，如果需要再次监听该文件描述符的话，需要再次把它加入到 EPOLL 队列中。

```c++
listenEvent_ = EPOLLRDHUP;
connEvent_ = EPOLLRDHUP | EPOLLONESHOT;
```

EPOLLONESHOT标志是一种特殊的epoll行为模式，用于确保一个socket连接上的事件在任何时刻只被一个线程处理。设置了EPOLLONESHOT后，一旦epoll报告了某个事件，该事件会被自动从epoll监听集合中移除，直到应用程序再次显式地重新将这个事件添加到epoll集合中。这样做主要是为了防止多个线程同时处理同一个socket的相同事件，从而避免竞态条件和不一致的情况。

为什么只对connEvent_设置EPOLLONESHOT：

- 连接处理复杂性：对于服务器来说，处理客户端的连接通常涉及读取数据、处理请求和发送响应等多个步骤。这些步骤可能需要更多的状态管理和更复杂的处理逻辑，尤其是在多线程环境中。使用EPOLLONESHOT可以确保连接处理的独占性，避免多个线程同时操作同一个连接造成数据错乱。
- 监听套接字与连接套接字的区别：listenEvent_通常用于监听套接字，它主要负责接受新的连接请求。监听套接字的事件（新的连接请求）相对简单，不涉及复杂的状态或数据处理，直接在主线程中处理，因此通常不需要EPOLLONESHOT。一旦接受了新的连接，相应的连接套接字（由 connEvent_管理）将负责后续的数据交换和更复杂的交互。
- 性能考虑：如果监听套接字也使用EPOLLONESHOT，那么每处理完一个新的连接请求后，服务器必须显式地重新将监听事件添加到epoll集合中。这会增加额外的系统调用开销，并可能降低服务器接受新连接的能力。
  
### Linger

```c++
optLiner.l_onoff = 1;
optLiner.l_linger = 1;
```

当设置了这样的linger选项后，套接字的关闭行为会按照以下方式改变：

- 如果有未发送完的数据在套接字的发送缓冲区中：关闭套接字的操作（例如调用 close()）不会立即返回。相反，操作系统会延迟套接字的实际释放，最多延迟l_linger指定的秒数。在这个例子中，系统会最多延迟1秒。
- 在这1秒内：操作系统会尝试继续发送缓冲区中的数据，并等待网络对端确认。
- 如果在1秒内数据成功发送并得到确认：套接字正常关闭，close()调用返回。
- 如果1秒后仍有数据未被发送或未得到确认：close()调用将返回，未发送完的数据可能会丢失，套接字被强制关闭。