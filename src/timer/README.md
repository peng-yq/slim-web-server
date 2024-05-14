## timer

使用小根堆实现的定时器，关闭超时的非活动连接。

- int id：连接套接字的文件描述符号
- Timestamp expires：到期时间
- TimeoutCallBack cb：回调函数

### usecase
```c++
#include <iostream>
#include <thread>
#include "timer.h"  

void timeoutHandler() {
    std::cout << "Connection timed out. Closing the connection..." << std::endl;
}

int main() {
    Timer timer;

    // 添加三个定时器，模拟三个不同的连接超时
    timer.Add(1, 1000, timeoutHandler);  // ID 1, 超时时间 1000 毫秒
    timer.Add(2, 1500, timeoutHandler);  // ID 2, 超时时间 1500 毫秒
    timer.Add(3, 2000, timeoutHandler);  // ID 3, 超时时间 2000 毫秒

    // 模拟服务器运行，定期检查定时器
    for (int i = 0; i < 5; ++i) {
        std::cout << "Server running, checking timers..." << std::endl;
        timer.Tick();  // 检查并执行所有到期的定时器
        int nextTick = timer.GetNextTick();
        if (nextTick > 0) {
            std::cout << "Next timer will tick in " << nextTick << " milliseconds." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(nextTick));  // 等待下一个定时器到期
        }
    }
    return 0;
}
```