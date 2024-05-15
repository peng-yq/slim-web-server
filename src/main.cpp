#include <unistd.h>
#include "server/web_server.h"

/* 端口 ET模式 timeoutMs 优雅退出  */
/* Mysql配置 */
/* 连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */

int main() {
    WebServer server (
        1316, 3, 60000, false,
        3306, "root", "12345678", "slimwebserver",
        12, 6, true, 0, 1024);
    server.Start();
}