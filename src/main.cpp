#include <unistd.h>
#include "server/web_server.h"

/* listenPort, ET mode, timeoutMs for close connection, socket graceful exit (Linger) */
/* Mysql configuration (port, user name, password, database name) */
/* size of sql connection pools, size of thread pools, enable log, log level, log asynchronous queue capacity (0 means no async) */

/*ET mode*/
/* 0: Both listening and connection events are LT*/
/* 1: The listening event is LT and the connection event is ET*/
/* 2: The listening event is ET and the connection event is LT*/
/* 3: Both listening and connection events are ET*/

/*Log level*/
/* 0: Debug, Info, Warn, Error*/
/* 1: Info, Warn, Error*/
/* 2: Warn, Error*/
/* 3: Error*/

int main() {
    WebServer server (
        1316, 3, 60000, false,
        3306, "root", "12345678", "slimwebserver",
        12, 6, true, 0, 1024);
    server.Start();
}