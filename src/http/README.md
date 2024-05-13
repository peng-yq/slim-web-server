## http

静态成员函数只能访问静态成员变量或调用其他的静态成员函数；它们不能访问类的非静态成员变量或调用非静态成员函数。

常量成员函数 (std::string HttpRequest::path() const):这个版本的 path() 函数是常量的，意味着它保证不会修改任何成员变量。它返回 path_ 的值的副本。因为返回的是副本，所以调用者无法通过这个副本修改原始数据。这个函数可以在常量对象上调用。

非常量成员函数 (std::string& HttpRequest::path()):这个版本的 path() 函数不是常量的，允许它修改成员变量。它返回 path_ 的引用，这意味着调用者可以通过这个引用直接修改 path_。这个函数只能在非常量对象上调用。

使用场景：

- 当你需要读取 path_ 值，但不需要修改它时，可以在常量和非常量对象上使用常量版本的函数。
- 当你需要修改 path_ 的值时，你应该使用非常量版本的函数，这只能在非常量对象上进行。

### HTTP GET 请求示例

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

这种类型的 HTTP 请求非常常见，特别是在浏览网页时。浏览器会发送类似的请求来获取网页内容，并通过 `Connection: keep-alive` 头部指示服务器保持连接，这样浏览器就可以快速连续请求网页上的其他资源，如图片、CSS 文件和 JavaScript 文件，无需每次都重新建立连接。

### HTTP POST 请求示例

下面是一个使用 POST 方法的 HTTP 1.1 请求示例。POST 请求通常用于向服务器提交数据，如表单数据、文件上传等。在这个示例中，我们将模拟一个用户通过表单提交用户名和密码的情况。

**请求行**:
```
POST /login HTTP/1.1
```

- `POST`：这是 HTTP 请求方法，用于向服务器提交数据。
- `/login`：这是数据提交到的服务器上的资源路径。
- `HTTP/1.1`：这指明了使用的 HTTP 版本。

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

### HTTP 响应示例

HTTP 响应是服务器在接收到客户端的 HTTP 请求后返回的数据。一个 HTTP 响应包括状态行、响应头部和响应体。下面是一个典型的 HTTP 响应示例：假设客户端请求一个网页，服务器的响应可能如下：

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
   - `HTTP/1.1 200 OK`：这一行表明使用的 HTTP 版本为 1.1，状态码为 200，表示请求成功处理，"OK" 是状态消息。

2. **响应头部**：
   - `Date: Thu, 11 May 2024 12:00:00 GMT`：表示响应生成的日期和时间。
   - `Server: Apache/2.4.1 (Unix)`：描述了服务器的软件信息。
   - `Last-Modified: Wed, 10 May 2024 23:11:55 GMT`：页面的最后修改时间。
   - `Content-Type: text/html; charset=UTF-8`：响应内容的类型和字符编码。
   - `Content-Length: 1234`：响应体的长度，单位是字节。
   - `Connection: close`：指示完成本次响应后关闭连接。

3. **响应体**：
   - 包含具体的内容，这里是一个简单的 HTML 页面。它展示了一个标题和一个段落。

这个示例展示了一个完整的 HTTP 响应，包括它的结构和每个部分的含义。服务器根据请求的不同，响应可以包含不同的头部信息和体内容。

- to do：密码是明文存储