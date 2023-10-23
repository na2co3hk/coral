#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include<memory>

#include"string_view"
#include"../tcp_client.hpp"
#include"http_request.hpp"
#include"http_response.hpp"

namespace coral {

class HTTPClient {
public:
    HTTPClient(const std::string_view host, const uint16_t port):
       client_(new TcpClient(host, port)) {};

    AUTO_GET_SET(client_, Client);

    void GET(Request& req) {
        buf += req.serialize();
        int saveErrno;
        client_->Send(&buf);
    }

    void POST(Request& req) {
        buf += req.serialize();
        int saveErrno;
        client_->Send(&buf);
    }

    void PUT(Request& req) {
        buf += req.serialize();
        int saveErrno;
        client_->Send(&buf);
    }

    void Delete(Request& req) {
        buf += req.serialize();
        int saveErrno;
        client_->Send(&buf);
    }

private:
    std::shared_ptr<TcpClient> client_;
    Buffer buf;
};

} //namespace coral

#endif //HTTP_CLIENT_HPP
