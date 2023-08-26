#ifndef AWAITER_HPP
#define AWAITER_HPP

//����
//�����޸�
//https://github.com/franktea/coro_epoll_kqueue/blob/main/awaiters.h

#include <type_traits>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include "awaitable.hpp"

#include "../network/socket.h"
#include "../network/io_context.h"

namespace coral {

//��suspend���첽ϵͳ������
template<typename Syscall, typename ReturnType>
class AsyncSyscall {
public:
    AsyncSyscall() : suspended_(false) {}

    bool await_ready() const noexcept { return false; }

    bool await_suspend(std::coroutine_handle<> h) noexcept {
        static_assert(std::is_base_of_v<AsyncSyscall, Syscall>);
        handle_ = h;
        value_ = static_cast<Syscall*>(this)->Syscall();
        suspended_ = value_ == -1 && (errno == EAGAIN || errno == EWOULDBLOCK);
        if (suspended_) {
            // ����ÿ��������coroutine handle��recv/send���ʵ���epoll�¼������������������
            static_cast<Syscall*>(this)->SetCoroHandle();
        }
        return suspended_;
    }

    ReturnType await_resume() noexcept {
       
        if (suspended_) {
            value_ = static_cast<Syscall*>(this)->Syscall();
        }
        return value_;
    }
protected:
    bool suspended_;
    // ��ǰawaiter����Э�̵�handle����Ҫ���ø�socket��coro_recv_����coro_send_����д����
    // handle_�����ڹ��캯�������õģ�����������Ĺ��캯����Ҳ�޷���ȡ��������await_suspend�Ժ��������
    std::coroutine_handle<> handle_;
    ReturnType value_;
};

class Socket;

class Accept : public AsyncSyscall<Accept, int> {
public:
    Accept(Socket* socket) : AsyncSyscall{}, socket_(socket) {
        socket_->io_context_.WatchRead(socket_);
  
    }

    ~Accept() {
        socket_->io_context_.UnwatchRead(socket_);
      
    }

    int Syscall() {
        struct sockaddr_storage addr;
        socklen_t addr_size = sizeof(addr);
      
        return ::accept(socket_->fd_, (struct sockaddr*)&addr, &addr_size);
    }

    void SetCoroHandle() {
        socket_->coro_recv_ = handle_;
    }
private:
    Socket* socket_;
    void* buffer_;
    std::size_t len_;
};

class Send : public AsyncSyscall<Send, ssize_t> {
public:
    Send(Socket* socket, void* buffer, std::size_t len) : AsyncSyscall(),
        socket_(socket), buffer_(buffer), len_(len)
    {
        socket_->io_context_.WatchWrite(socket_);  
    }
    ~Send() {
        socket_->io_context_.UnwatchWrite(socket_);
    }

    ssize_t Syscall() {
        return ::send(socket_->fd_, buffer_, len_, 0);
    }

    void SetCoroHandle() {
        socket_->coro_send_ = handle_;
    }

private:
    Socket* socket_;
    void* buffer_;
    std::size_t len_;
};

class Recv : public AsyncSyscall<Recv, int> {
public:
    Recv(Socket* socket, void* buffer, size_t len) : AsyncSyscall(),
        socket_(socket), buffer_(buffer), len_(len)
    {
        socket_->io_context_.WatchRead(socket_);
    }

    ~Recv() {
        socket_->io_context_.UnwatchRead(socket_);
    }

    ssize_t Syscall() {
        return ::recv(socket_->fd_, buffer_, len_, 0);
    }

    void SetCoroHandle() {
        socket_->coro_recv_ = handle_;
    }

private:
    Socket* socket_;
    void* buffer_;
    std::size_t len_;
};

} //namespace coral

#endif //AWAITER_HPP