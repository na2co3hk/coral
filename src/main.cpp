#include<iostream>
#include<string>

#include"log/logstream.hpp"
#include"test/unit_test.hpp"

#include"base/json.h"
#include"base/basic_buffer.hpp"
#include"base/basic_data_structure.hpp"

#include"coroutine/awaitable.hpp"
#include"coroutine/awaiter.hpp"
#include"coroutine/chan.hpp"

#include"network/socket.h"
#include"network/io_context.h"
#include"network/inet_address.hpp"
#include"network/http/http_server.hpp"

using namespace std;

TEST_CASE(inet_test) {
	coral::InetAddress addr(8080);
	ASSERT_EQ(8080, addr.toPort());
	ASSERT_EQ("0.0.0.0", addr.toIp());
}

TEST_CASE(skip_list_test) {
	coral::SkipList<int>skl;
	LOG_INFO << "skiplist construct";
	skl.add(1);
	skl.add(2);
	skl.add(3);
	ASSERT_EQ(false, skl.search(0));
	skl.add(4);
	ASSERT_EQ(true, skl.search(1));
	ASSERT_EQ(false, skl.erase(0));
	ASSERT_EQ(true, skl.erase(1));
	ASSERT_EQ(false, skl.search(1));
}

//TEST_CASE(chan_test) {
//	auto ch = make(chan, int, 3);
//	using namespace coral;
//
//	auto t1 = [&ch]()->task<> {
//		co_await (ch << 13);
//		co_await (ch << 16);
//	}();
//
//	auto t2 = [&ch]()->task<int> {
//		int i;
//		co_await(ch >> i);
//		co_return i;
//	}();
//}

//using namespace coral;
//
//awaitable<bool> inside_loop(Socket& socket) {
//    char buffer[1024] = { 0 };
//    ssize_t recv_len = co_await socket.recv(buffer, sizeof(buffer));
//    ssize_t send_len = 0;
//    while (send_len < recv_len) {
//        ssize_t res = co_await socket.send(buffer + send_len, recv_len - send_len);
//        if (res <= 0) {
//            co_return false;
//        }
//        send_len += res;
//    }
//
//    
//    if (recv_len <= 0) {
//        co_return false;
//    }
//    co_return true;
//}
//
//awaitable<> echo_socket(std::shared_ptr<Socket> socket) {
//    for (;;) {
//        
//        bool b = co_await inside_loop(*socket);
//        if (!b) break;
//       
//    }
//}
//
//awaitable<> accept(Socket& listen) {
//    for (;;) {
//        auto socket = co_await listen.accept();
//        auto t = echo_socket(socket);
//        t.resume();
//    }
//}

using namespace coral;
using json = nlohmann::json;

int main() {

	Router& r = Router::instance();
	r.GET("/", [](Request& req, Response& rsp) {
		rsp.setPath("coarl.json");
		json hello = {
			{"msg", "hello!"},
			{"code", 200}
		};
		rsp.write(hello.dump());
	});

	r.GET("/args", [](Request& req, Response& rsp) {
		rsp.setPath("coarl.json");
		std::string name = req.getParams("name");
		std::string age = req.getParams("age");
		json hello = {
			{"msg", "hello!"},
			{"code", 200},
			{"name", name},
			{"age", age}
		};

		rsp.write(hello.dump());
	});

	IoContext ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return RUN_ALL_TESTS();
}
