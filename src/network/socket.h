#ifndef SOCKET_H
#define SOCKET_H

#include <memory>
#include <coroutine>

#include "../coroutine/awaitable.hpp"

namespace coral {

class Send;
class Recv;
class Accept;

class Socket;
class Context;

class Socket {
public:
    Socket(std::string_view port, Context& io_context);

    Socket(const Socket&) = delete;
    Socket(Socket&& socket) :
        io_context_(socket.io_context_),
        fd_(socket.fd_),
        io_state_(socket.io_state_) {
        socket.fd_ = -1;
    }

    ~Socket();

    awaitable<std::shared_ptr<Socket>> accept(Context& ioLoop);

    Recv recv(void* buffer, std::size_t len);

    Send send(void* buffer, std::size_t len);

    bool ResumeRecv();

    bool ResumeSend();

    void shutdownWrite();
private:
    friend Accept;
    friend Recv;
    friend Send;
    friend Context;

    explicit Socket(int fd, Context& io_context);
private:
    Context& io_context_;
    int fd_ = -1;
    int32_t io_state_ = 0; // 当前已经注册的可读可写等事件，epoll需要用modify所以需要将旧的事件保存起来
    // 因为可能有两个协程同时在等待一个socket，所以要用两个coroutine_handle来保存。
    std::coroutine_handle<> coro_recv_; // 接收数据的协程
    std::coroutine_handle<> coro_send_; // 发送数据的协程
};

} //namespace coral

#endif //SOCKET_H