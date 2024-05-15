# slim-web-server

slim-web-server: a high-performance, lightweight web server, also the beginning of my modern c++ journey. 

## feature

- 利用标准库容器封装char，实现动态增长的字节流缓冲区；

- 利用锁、条件变量和队列实现单生产者消费者的阻塞队列；

- 利用单例模式与阻塞队列实现支持异步和同步的日志系统，记录服务器运行状态；

- 基于小根堆实现的定时器，关闭超时的非活动连接，节约系统资源；

- 利用RAII机制实现了数据库连接池，减少频繁打开和关闭数据库连接的开销，提高数据库操作的效率；

- 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求、用户注册和登录；

- 利用IO复用技术Epoll与线程池实现多线程的Reactor高并发模型；

## how-to-use

1. Requirements

     - Linux
     - C++14
     - MySql

2. Git Clone this repo

    ```shell
    git clone https://github.com/peng-yq/slim-web-server.git
    ```

2. Configure Mysql

   - 下载mysql
   - 设置用户和密码
   - 并创建数据库和表

3. Make
   
    编译之前需要配置main.cpp中的相关参数，例如mysql配置。

    ```shell
    # pwd is path/to/slim-web-server
    make
    ```
4. Run
   
    ```shell
    # pwd is path/to/slim-web-server
    ./slim-web-server
    ```
5. WebBench Test
   
   测试前需要先编译WebBench。

    ```shell
    # pwd is path/to/slim-web-server/webbench-1.5
    make install
    webbench -c 100 -t 10 http://ip:port/
    webbench -c 1000 -t 10 http://ip:port/
    webbench -c 5000 -t 10 http://ip:port/
    webbench -c 10000 -t 10 http://ip:port/
    ```

## to-do

- [ ] 使用配置文件或CLI启动服务器，而非每次修改配置都需重新编译
- [ ] 数据库用户密码加密存储
- [ ] 循环缓冲区
- [ ] 单元测试
- [ ] 视频流传输性能
- [ ] redis
- [ ] HTTPS

## 致谢

> 虽然是所谓的“烂大街”项目，但对于C++初学者而言，是一个极佳的现代C++入门项目。完整认真的去做一遍会学到很多，并且能将所谓的“八股”和C++特性进行实际运用，加深理解。

- [markparticle](https://github.com/markparticle/WebServer)

相较于原版项目的一些改动：

  1. 代码风格更加统一规范
  2. 对部分模块进行了重新划分
  3. 对每一个模块编写了较为完整的README
  4. 修复了一些明显的bug