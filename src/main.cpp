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

using namespace coral;
using json = nlohmann::json;

struct LogAspect {

	void Before(Request& req, Response& rsp) {
		rsp.write("before aspect\r\n");
	}

	void After(Request& req, Response& rsp) {
		rsp.write("after aspect\r\n");
	}
};

int main() {

	Router& r = Router::instance();
	r.GET("/", [](Request& req, Response& rsp) {
		rsp.setPath("coarl.json");
		Cookie cookie("name", "huake");
		cookie.setPath("path");
		cookie.setMaxAge(60);
		cookie.setHttpOnly(true);
		cookie.setSecure(true);
		rsp.setCookie(cookie);

		json yes = {
			{"msg", "yes"},
			{"code", 200}
		};
		rsp.write(yes.dump());
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

	r.GET("/aspect", [](Request& req, Response& rsp) {
		rsp.setPath("coral.txt");
		rsp.write("this is AOP test\r\n");
	}, LogAspect{});

	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return 0;
}
