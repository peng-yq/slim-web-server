## block_deque

线程安全的阻塞双端队列（BlockDeque）实现，支持在多线程环境中进行同步访问。采用生产者-消费者模式，确保数据的安全生产和消费，适合用于任务调度、消息队列等场景。Slim Web Server项目中主要用于服务日志的异步写，日志服务作为生产者往队列中推送需要记录的日志条目，异步写线程作为消费者取出写入日志文件。

**主要特性**

- 线程安全：通过互斥锁（mutex）和条件变量（condition_variable）确保在多线程环境下的互斥和同步。
- 阻塞操作：支持阻塞的数据插入和移除操作，当队列满或空时，生产者和消费者线程将会等待。
- 条件通知：使用条件变量来实现线程的正确唤醒和阻塞，优化了线程间的协调。
- 动态控制：允许在运行时清空队列、关闭队列，并通知所有阻塞的线程。
- 容量限制：队列大小有上限，防止无限制增长导致的资源耗尽。

**互斥锁和条件变量**

- 互斥锁：保护队列的内部状态，确保在多线程操作时数据的一致性和完整性。
- 条件变量：用于在队列为空时阻塞消费者线程，在队列满时阻塞生产者线程。当条件得到满足时，相应的线程会被唤醒。

**生产者-消费者模式**

> 这里实现的是单生产者单消费者

生产者：负责向队列中添加新的元素，并通知阻塞的消费者。如果队列已满，则生产者线程会阻塞，直到队列中有空间可用。
消费者：从队列中取出元素进行处理，并通知阻塞的生产者。如果队列为空，消费者线程会阻塞，直到队列中有新的元素可用。

### usecase

```c++
#include "block_deque.h"
#include <iostream>
#include <thread>

void producer(BlockDeque<int>& deque) {
    for (int i = 0; i < 10; ++i) {
        deque.push_back(i);
        std::cout << "Produced: " << i << std::endl;
    }
}

void consumer(BlockDeque<int>& deque) {
    int item;
    while (deque.pop_front(item)) {
        std::cout << "Consumed: " << item << std::endl;
    }
}

int main() {
    BlockDeque<int> deque(5);  // 初始化一个最大容量为5的队列

    std::thread prod(producer, std::ref(deque));
    std::thread cons(consumer, std::ref(deque));

    prod.join();
    cons.join();

    return 0;
}
```