#include <memory>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <string_view>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include "socket.h"
#include "io_context.h"
#include "../coroutine/awaiter.hpp"

namespace coral {

Socket::Socket(std::string_view port, IoContext& io_context) :
    io_context_(io_context) {
    struct addrinfo hints, * res;

    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC; // use IPv4 or v6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me 

    getaddrinfo(NULL, port.data(), &hints, &res);
    fd_ = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int opt;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    if (::bind(fd_, res->ai_addr, res->ai_addrlen) != 0) {
        throw std::runtime_error{ "bind error" };
    }
    ::listen(fd_, 8);
    fcntl(fd_, F_SETFL, O_NONBLOCK);
    io_context_.Attach(this);
    io_context_.WatchRead(this);
}

Socket::~Socket() {
    if (fd_ == -1) return;
    io_context_.Detach(this);
       
    ::close(fd_);
}

awaitable<std::shared_ptr<Socket>> Socket::accept(IoContext& ioLoop) {
    int fd = co_await Accept{ this };
    if (fd == -1) {
        throw std::runtime_error{ "accept error" };
    }
        
    co_return std::shared_ptr<Socket>(new Socket{ fd, ioLoop });
}

Recv Socket::recv(void* buffer, std::size_t len) {
    return Recv{ this, buffer, len };
}

Send Socket::send(void* buffer, std::size_t len) {
    return Send{ this, buffer, len };
}

bool Socket::ResumeRecv() {
    if (coro_recv_) {
        coro_recv_.resume();
    }
    return true;
}

bool Socket::ResumeSend() {
    if (coro_send_) {
        coro_send_.resume();
    }
    return true;
}

Socket::Socket(int fd, IoContext& io_context) : io_context_(io_context),
    fd_(fd) {
    fcntl(fd_, F_SETFL, O_NONBLOCK);
    io_context_.Attach(this);
}

void Socket::shutdownWrite()
{
    /**
     * 关闭写端，但是可以接受客户端数据
     */
    if (::shutdown(fd_, SHUT_WR) < 0)
    {
        throw std::runtime_error{ "shutdown error" };
    }
}

} //namespace coral