#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include<string_view>
#include<functional>

#include"../base/basic_buffer.hpp"
#include"../log/logger.hpp"
#include"../coroutine/awaiter.hpp"
#include"../coroutine/awaitable.hpp"

namespace coral{

class TcpClient{
public:
    using Handler = std::function<void(coral::Buffer &)>;

    TcpClient(const std::string_view host, const uint16_t port):
       fd_(-1),
       host_(host),
       port_(port)
    {
        fd_ =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(fd_ == -1){
            coral::log.Error("Client fd create error!");
            conneted_ = false;
        }

        struct sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(port);
        saddr.sin_addr.s_addr = inet_addr(host.data());
        int ret = connect(fd_, (struct sockaddr*)&saddr, sizeof saddr);
        if(ret == -1){
            coral::log.Error("Connection error!");
            conneted_ = false;
        }
        conneted_ = true;
    }

    ~TcpClient() {
        close(fd_);
    }

    bool isConnected() const {
        return conneted_;
    }

    int Send(coral::Buffer* buf, int* saveErrno = nullptr) {
        int n = buf->writeFd(fd_, saveErrno);
    }

    int Recv(coral::Buffer* buf, int* saveErrno = nullptr) {
        int n = buf->readFd(fd_, saveErrno);
        if(n > 0) {
            if(messageHandler_){
                messageHandler_(*buf);
            }
        }
        else if (n == 0) {
            conneted_ = false;
            coral::log.Error("Disconneted!");
        }
        else {
            coral::log.Error("recv() function call error!");
        }
        return n;
    }

    void setMessageHandler(Handler massageHandler) {
        messageHandler_ = massageHandler;
    }

private:

    bool conneted_;
    int fd_;
    std::string_view host_;
    uint16_t port_;
    Handler messageHandler_;
};

}

#endif //TCP_CLIENT_HPP
