#include<iostream>
#include<string>

#include"log/logger.hpp"
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

TEST_CASE(log_test) {
	coral::Logger& crlog = coral::Logger::instance();
	crlog.Info("this is a normal information");
	crlog.Warn("this is a warning");
	crlog.Error("this is a error");
	crlog.Fatal("this is a fatal error");
}

using namespace coral;
using json = nlohmann::json;

struct LogAspect {

	bool Before(Request& req, Response& rsp) {
		coral::Logger& log = coral::Logger::instance();
		log.Info("new connection!");
		return true;
	}

	void After(Request& req, Response& rsp) {
		coral::Logger& log = coral::Logger::instance();
		log.Info("disconneted!");
	}
};

int main() {

    coral::Logger& log = coral::Logger::instance();
	Router& r = Router::instance();

    log.Info("Http-server start!");
	r.GET("/login/{:id}/password/{:pwd}", [](Request& req, Response& rsp) {
		std::string id = req.getParam("id");
		std::string pwd = req.getParam("pwd");
		std::string token = req.getQuery("token");
		json msg = {
			{"code", 200},
			{"username", id},
			{"password", pwd},
			{"token", token},
		};
		rsp.json(msg.dump());
	}, LogAspect{});

	r.POST("/register", [](Request& req, Response& rsp) {
		json msg = {
			{"code", 200},
			{"msg", "register successfuly!"},	
		};
		rsp.json(msg.dump());
	});

	r.PUT("/update", [](Request& req, Response& rsp) {
		json msg = {
			{"code", 200},
			{"msg", "update successfully!" },
		};
		rsp.json(msg.dump());
	}, LogAspect{});

	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();

	
	return 0;
}
