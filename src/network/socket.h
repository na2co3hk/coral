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
class IoContext;

class Socket {
public:
    Socket(std::string_view port, IoContext& io_context);

    Socket(const Socket&) = delete;
    Socket(Socket&& socket) :
        io_context_(socket.io_context_),
        fd_(socket.fd_),
        io_state_(socket.io_state_) {
        socket.fd_ = -1;
    }

    ~Socket();

    awaitable<std::shared_ptr<Socket>> accept(IoContext& ioLoop);

    Recv recv(void* buffer, std::size_t len);

    Send send(void* buffer, std::size_t len);

    bool ResumeRecv();

    bool ResumeSend();

    void shutdownWrite();
private:
    friend Accept;
    friend Recv;
    friend Send;
    friend IoContext;

    explicit Socket(int fd, IoContext& io_context);
private:
    IoContext& io_context_;
    int fd_ = -1;
    int32_t io_state_ = 0; // ��ǰ�Ѿ�ע��Ŀɶ���д���¼���epoll��Ҫ��modify������Ҫ���ɵ��¼���������
    // ��Ϊ����������Э��ͬʱ�ڵȴ�һ��socket������Ҫ������coroutine_handle�����档
    std::coroutine_handle<> coro_recv_; // �������ݵ�Э��
    std::coroutine_handle<> coro_send_; // �������ݵ�Э��
};

} //namespace coral

#endif //SOCKET_H