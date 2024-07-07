# slim-web-server

slim-web-server: a high-performance, lightweight web server, also the beginning of my modern c++ journey. 

## feature

- 利用标准库容器封装char，实现动态增长的字节流缓冲区；

- 利用锁、条件变量和队列实现单生产者消费者的阻塞队列；

- 利用单例模式与阻塞队列实现支持异步和同步的日志系统，记录服务器运行状态；

- 基于小根堆实现的定时器，关闭超时的非活动连接，节约系统资源；

- 利用RAII机制实现了数据库连接池，减少频繁打开和关闭数据库连接的开销，提高数据库操作的效率；

- 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求、用户注册和登录；

- 利用IO复用技术Epoll与线程池实现多线程的Reactor高并发模型，使用webbench-1.5进行压力测试可以实现上万的QPS；

**webbench测试环境与结果**

<img src="https://cdn.jsdelivr.net/gh/peng-yq/Gallery/202406051255703.png"/>

<img src="https://cdn.jsdelivr.net/gh/peng-yq/Gallery/202406071038268.png">

> 一开始在WSL2中进行测试，效果很不理想。QPS很低，而且请求成功率也很低，以为是代码出问题了（因为我机子性能肯定没问题），后面换成了虚拟机才正常，看来WSL还是只能小打小闹。

> 关于性能测试：Webbench是一个在linux下使用的非常简单的网站压测工具。它使用fork()模拟多个客户端同时访问我们设定的URL，测试网站在压力下工作的性能，最多可以模拟3万个并发连接去测试网站的负载能力。因此测出来的数据只能说见仁见智，我后面又用apache-jmeter 5.6.3进行了测试，测出来的数据肯定是没有这么好看的，但也算能在高压下稳定，“高性能”的运行。当然还是那句话，脱离业务、机器和请求量谈高性能都是耍流氓。

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
  4. 重构了Makfile，优化编译过程
  5. 修复了一些明显的bug
