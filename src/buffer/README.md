## Buffer

动态字节缓冲区，用于处理数据的临时存储，优化数据的读取和发送过程。

> buffer的实现中使用了互斥锁来实现线程安全，但在slim web server的实现中，buffer这里可以不要互斥锁，因为buffer都是封装在其它类中使用，其它类均使用互斥同步机制确保了线程安全。

**主要特性**

- 线程安全：通过互斥锁保证在多线程环境下缓冲区的线程安全性。
- 动态扩展：根据需要动态调整缓冲区大小，以适应不同大小的数据负载。
- 高效的内存管理：优化内存使用，减少数据拷贝，提高性能。
- 支持多种数据操作：提供多种方法来读取、写入、追加和清除缓冲区中的数据。

**缓冲区操作**

- 读写指针管理：通过管理读写指针来优化数据的处理，避免不必要的数据复制。
- 自动扩容：当可写空间不足时，缓冲区能自动扩容以存储更多数据。
- 数据追加：支持多种数据类型的追加，包括字符串、原始数据和其他缓冲区的内容。

### usecase

```c++
#include "buffer.h"

int main() {
    Buffer buffer(2048);  // 初始化一个大小为2048字节的缓冲区
    int fd = 0;  // 假设已经有一个打开的文件描述符
    int err = 0;

    // 从文件描述符读取数据到缓冲区
    ssize_t bytesRead = buffer.ReadFromFd(fd, &err);
    if (bytesRead > 0) {
        // 处理读取到的数据
        std::cout << "读取到的数据: " << buffer.RetrieveAllAsString() << std::endl;
    }

    // 将字符串追加到缓冲区
    std::string data = "Hello, World!";
    buffer.Append(data);

    // 将缓冲区数据写入文件描述符
    ssize_t bytesWritten = buffer.WriteToFd(fd, &err);
    if (bytesWritten > 0) {
        std::cout << "成功写入数据到文件描述符" << std::endl;
    }

    return 0;
}
```