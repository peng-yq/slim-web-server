## http

HTTP连接处理模块，负责处理HTTP请求和响应。支持多线程环境下的高效HTTP请求解析和响应生成，适用于处理静态资源请求、用户登录验证等功能。

**主要特性**

- 高效的请求解析：使用正则表达式和状态机解析HTTP GET和POST请求，包括请求行、头部字段和消息体。
- 动态响应生成：根据请求动态生成HTTP响应，包括状态行、响应头和响应体。
- 文件映射支持：使用内存映射技术优化文件访问速度，适用于静态文件服务。
- 连接管理：支持长连接，根据HTTP/1.1的Connection: keep-alive管理TCP连接。
- 并发用户统计：通过原子操作统计并发连接数，确保数据的准确性。

**HttpConn类**

封装了HttpRequest类和HttpResponse类，负责单个HTTP连接的管理，包括初始化连接、读写数据、处理请求和生成响应。

**HttpRequest类**

解析客户端发来的HTTP请求，包括请求行、请求头和消息体。支持解析URL编码的POST数据。

**HttpResponse类**

根据HttpRequest的解析结果生成HTTP 应。支持错误处理，能够根据不同的错误码返回不同的错误页面。

### HTTP GET请求示例

一个完整的 HTTP 请求示例包括**请求行、请求头部以及可选的请求体**。下面是一个使用 GET 方法的 HTTP 1.1 请求示例，该请求可能用于从服务器获取一个 HTML 页面，同时指定连接应保持活跃（Keep-Alive）。

**请求行**:
```
GET /index.html HTTP/1.1
```

- `GET`：这是 HTTP 请求方法，用于请求访问服务器上的资源。
- `/index.html`：这是请求的资源的路径。
- `HTTP/1.1`：这指明了使用的 HTTP 版本。

**请求头部**:

```html
Host: www.example.com
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate
Connection: keep-alive
```

- `Host: www.example.com`：这指定了请求将发送到的服务器。
- `User-Agent: Mozilla/5.0 ... Safari/537.36`：这提供了关于发出请求的客户端软件的信息，通常用于统计和兼容性处理。
- `Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8`：这告诉服务器客户端能够接收哪些媒体类型。
- `Accept-Language: en-US,en;q=0.5`：这指明了客户端优先接收的语言。
- `Accept-Encoding: gzip, deflate`：这告诉服务器客户端支持哪些压缩格式。
- `Connection: keep-alive`：这指示服务器保持连接打开，以便客户端可以通过同一连接发送进一步的请求。

**请求体**:

```
(此处没有请求体，因为GET请求通常不包括请求体)
```

这种类型的HTTP请求非常常见，特别是在浏览网页时。浏览器会发送类似的请求来获取网页内容，并通过 `Connection: keep-alive` 头部指示服务器保持连接，这样浏览器就可以快速连续请求网页上的其他资源，如图片、CSS文件和JavaScript文件，无需每次都重新建立连接。

### HTTP POST请求示例

下面是一个使用POST方法的HTTP 1.1请求示例。POST请求通常用于向服务器提交数据，如表单数据、文件上传等。在这个示例中，我们将模拟一个用户通过表单提交用户名和密码的情况。

**请求行**:
```
POST /login HTTP/1.1
```

- `POST`：这是HTTP请求方法，用于向服务器提交数据。
- `/login`：这是数据提交到的服务器上的资源路径。
- `HTTP/1.1`：这指明了使用的HTTP版本。

**请求头部**:
```
Host: www.example.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 27
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate
Connection: keep-alive
```

- `Host: www.example.com`：这指定了请求将发送到的服务器。
- `Content-Type: application/x-www-form-urlencoded`：这指明了发送的数据类型，表示表单数据被编码为键值对，如同查询字符串。
- `Content-Length: 27`：这指明了请求体的长度，必须正确指定以便服务器正确接收全部数据。
- `User-Agent: Mozilla/5.0 ... Safari/537.36`：这提供了关于发出请求的客户端软件的信息。
- `Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8`：这告诉服务器客户端能够接收哪些媒体类型。
- `Accept-Language: en-US,en;q=0.5`：这指明了客户端优先接收的语言。
- `Accept-Encoding: gzip, deflate`：这告诉服务器客户端支持哪些压缩格式。
- `Connection: keep-alive`：这指示服务器保持连接打开，以便客户端可以通过同一连接发送进一步的请求。

**请求体**:
```
username=johndoe&password=12345
```

- `username=johndoe&password=12345`：这是实际的数据部分，包含了表单中填写的用户名和密码。数据以键值对形式发送，每对键值用 `&` 符号分隔。

### HTTP响应示例

HTTP响应是服务器在接收到客户端的HTTP请求后返回的数据。一个HTTP响应包括状态行、响应头部和响应体。下面是一个典型的HTTP响应示例：假设客户端请求一个网页，服务器的响应可能如下：

```html
HTTP/1.1 200 OK
Date: Thu, 11 May 2024 12:00:00 GMT
Server: Apache/2.4.1 (Unix)
Last-Modified: Wed, 10 May 2024 23:11:55 GMT
Content-Type: text/html; charset=UTF-8
Content-Length: 1234
Connection: close

<html>
<head>
    <title>An Example Page</title>
</head>
<body>
    <h1>Hello, World!</h1>
    <p>This is an example of a simple HTML page with one paragraph.</p>
</body>
</html>
```

1. **状态行**：
   - `HTTP/1.1 200 OK`：这一行表明使用的HTTP版本为1.1，状态码为200，表示请求成功处理，"OK" 是状态消息。

2. **响应头部**：
   - `Date: Thu, 11 May 2024 12:00:00 GMT`：表示响应生成的日期和时间。
   - `Server: Apache/2.4.1 (Unix)`：描述了服务器的软件信息。
   - `Last-Modified: Wed, 10 May 2024 23:11:55 GMT`：页面的最后修改时间。
   - `Content-Type: text/html; charset=UTF-8`：响应内容的类型和字符编码。
   - `Content-Length: 1234`：响应体的长度，单位是字节。
   - `Connection: close`：指示完成本次响应后关闭连接。

3. **响应体**：
   - 包含具体的内容，这里是一个简单的HTML页面。它展示了一个标题和一个段落。

这个示例展示了一个完整的HTTP响应，包括它的结构和每个部分的含义。服务器根据请求的不同，响应可以包含不同的头部信息和体内容。

### usecase

```c++
#include "http_connect.h"

int main() {
    HttpConn conn;
    sockaddr_in addr;
    // 假设已经创建了套接字并接受了连接
    int sockFd = accept(...);

    // 初始化连接
    conn.Init(sockFd, addr);

    int saveErrno;
    // 读取数据
    if (conn.Read(&saveErrno) > 0) {
        // 处理请求
        if (conn.Process()) {
            // 发送响应
            conn.Write(&saveErrno);
        }
    }

    // 关闭连接
    conn.Close();

    return 0;
}
```
