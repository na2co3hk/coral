#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include<memory>
#include<thread>
#include<string>
#include<string_view>
#include<functional>

#include"socket.h"
#include"io_context.h"
#include"loop_threadpool.hpp"
#include"../log/logstream.hpp"
#include"../base/noncopyable.h"
#include"../coroutine/awaitable.hpp"
#include"../coroutine/awaiter.hpp"

namespace coral {

class TcpServer {
public:
	using SocketPtr = std::shared_ptr<Socket>;
    using Handler = std::function<void(coral::Buffer &)>;

	explicit TcpServer(const std::string_view& port, IoContext& ctx,
		const std::string_view& name) :
		acceptor_(port, ctx),
		ctx_(ctx),
		name_(name),
		threadPool_(3)
	{
		awaitable<>t = accept();
		t.resume();
	}

	void run() {
		threadPool_.start();
	}

    void setMessageHandler(Handler massageHandler) {
        messageHandler_ = massageHandler;
    }

    awaitable<bool> inside_loop(Socket& socket) {
        /*char buffer[1024] = { 0 };*/
        Buffer buffer(2048);
        ssize_t recv_len = co_await socket.recv(buffer.beginWrite(), buffer.writableBytes());
        buffer.moveWriter(recv_len);
        if (messageHandler_) {
            messageHandler_(buffer);
        }
        ssize_t send_len = 0;
        ssize_t total_len = buffer.readableBytes();
        while (send_len < total_len) {
            ssize_t res = co_await socket.send(buffer.peek() + send_len, total_len - send_len);
            buffer.moveReader(res);
            if (res <= 0) {
                co_return false;
            }
            send_len += res;
        }

        buffer.retrieveAll();
        if (total_len == send_len) {
            socket.shutdownWrite();
            co_return false; //断开连接
        }

        if (recv_len <= 0) {
            co_return false;
        }
            
        co_return true;
    }

    awaitable<> handle(std::shared_ptr<Socket> socket) {
        for (;;) {
            bool b = co_await inside_loop(*socket); 
            if (!b) break; //close Handler
        }
    }

    awaitable<> accept() {
        for (;;) {
            IoContext& ioLoop = threadPool_.getNextLoop();
            auto socket = co_await acceptor_.accept(ioLoop);//co_await调用是共用一个协程,也就是阻塞的时候回到构造函数
            auto t = handle(socket); //直接调用是产生一个新的协程
            t.resume();
        }
    }

private:

	Socket acceptor_;
	LoopThreadPool threadPool_;
	std::string name_;
	IoContext ctx_;

    Handler messageHandler_;
};

} //namespace coral

#endif //TCP_SERVER_HPP