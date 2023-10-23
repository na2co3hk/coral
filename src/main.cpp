#include<coral.h>

using namespace coral;
using json = nlohmann::json;

TEST_CASE(client_test) {
    //req test
    Request req;
    req.setMethod("GET");
    req.setPath("/");
    req.setHeader("Connection", "keep-alive");
    req.setBody("yes");
    coral::log.Debug(req.serialize() );

    //rsp test
    Buffer buf;
    buf += "HTTP/1.1 200 OK\r\n";
    buf += "Connection: keep-alive\r\n";
    buf += "keep-alive: max=6, timeout=120\r\n";
    buf += "\r\n";
    buf += "nihaoya\r\n";
    buf += "zhenbucuo";
    Response rsp(buf, true);
    rsp.deserialize();

    coral::log.Debug(rsp.getBody());
}

struct LogAspect {

	bool Before(Request& req, Response& rsp) {
		coral::log.Info("new connection!");
		return true;
	}

//	void After(Request& req, Response& rsp) {
//		coral::log.Info("disconneted!");
//	}
};

int main() {

	Router& r = Router::instance();

    coral::log.Info("Http-server start!");
//	r.GET("/login/{:id}/password/{:pwd}", [](Request& req, Response& rsp) {
//		std::string id = req.getParam("id");
//		std::string pwd = req.getParam("pwd");
//		std::string token = req.getQuery("token");
//		json msg = {
//			{"code", 200},
//			{"username", id},
//			{"password", pwd},
//			{"token", token},
//		};
//		rsp.json(msg.dump());
//	}, LogAspect{});
//
//	r.POST("/register", [](Request& req, Response& rsp) {
//		json msg = {
//			{"code", 200},
//			{"msg", "register successfuly!"},
//		};
//		rsp.json(msg.dump());
//	}, LogAspect{});
//
//	r.PUT("/update", [](Request& req, Response& rsp) {
//		json msg = {
//			{"code", 200},
//			{"msg", "update successfully!" },
//		};
//		rsp.json(msg.dump());
//	}, LogAspect{});
    r.GET("/", [](Request& req, Response& rsp){
        rsp.setPath(".json");
        rsp.write("hello! client");
    }, LogAspect{});

	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();

	return 0;
}