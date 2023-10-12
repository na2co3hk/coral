#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include"cookie.hpp"
#include"http_request.hpp"
#include"http_response.hpp"
#include"http_router.hpp"

#include"../../base/basic_buffer.hpp"
#include"../tcp_server.hpp"
#include"../../base/noncopyable.h"


namespace coral {

class HTTPServer : noncopyable {
public:
	HTTPServer(const std::string_view port, Context& ctx) :
		port_(port),
		server_(port, ctx, "HTTP-SERVER")
	{
		server_.setMessageHandler(std::bind(&HTTPServer::messageHandler, this, std::placeholders::_1));
	}

	std::string getPort() const {
		return port_;
	}

	void messageHandler(Buffer &buffer) {
			
		Request req(buffer);
		Response rsp(buffer, req.getErr(), req.isKeepAlive());

		Router& router = Router::instance();
		router.handleHTTPRequest(req, rsp);
		rsp.addStateLine();
		rsp.addHeader();
		rsp.addBody();
	}

	void run() {
		server_.run();
	}

private:
		
	std::string port_;
	TcpServer server_;
};

} //namespace coral

#endif //HTTP_SERVER_HPP