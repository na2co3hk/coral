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

	bool Before(Request& req, Response& rsp) {
		LOG_INFO << "new connection";
		json msg = {
			{"code", 404},
			{"msg", "no"}
		};
		rsp.json(msg.dump());
		return true;
	}

	void After(Request& req, Response& rsp) {
		LOG_INFO << "disconnection";
	}
};

struct CheckAspect {

	bool Before(Request& req, Response& rsp) {
		LOG_INFO << "check";
		return false;
	}
};

int main() {

	Router& r = Router::instance();
	r.GET("/", [](Request& req, Response& rsp) {
		json msg = {
			{"code", 200},
			{"msg", "ok"}
		};
		rsp.json(msg.dump());
	}, LogAspect{}, CheckAspect{});

	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return 0;
}
