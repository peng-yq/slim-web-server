## timer

使用小根堆实现的定时器，关闭超时的非活动连接。

别名：
- TimeoutCallBack：函数对象类型，可以接受任何可调用对象（函数指针、lambda表达式和函数对象），这些调用对象不接受参数并返回void（void()），用于回调函数
- HighResolutionClock：用于高精度的时间控制
- Milliseconds：毫秒
- Timestamp：表示一个具体的时刻