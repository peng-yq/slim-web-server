## timer

使用小根堆实现的定时器，关闭超时的非活动连接。

别名：
- TimeoutCallBack：函数对象类型，可以接受任何可调用对象（函数指针、lambda表达式和函数对象），这些调用对象不接受参数并返回void（void()），用于回调函数
- HighResolutionClock：用于高精度的时间控制
- Milliseconds：毫秒
- Timestamp：表示一个具体的时刻

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

    // 清理资源
    timer.Clear();
    return 0;
}
```