#include<coral.h>

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
    RUN_ALL_TESTS();
	Router& r = Router::instance();

    coral::log.Info("Http-server start!");
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
	}, LogAspect{});

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