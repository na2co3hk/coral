# Coral - Modern C++ HTTP server, based on C++20 coroutine

## About Coral
coral is a high-performance modern HTTP server that supports GET/POST requests and database interactions, file uploads (not yet developed), and more.

Here are its main features
* Excellent performance: based on C++20 coroutine, instead of the original callback mode, can perform asynchronous operations in a synchronous manner.
* Easy to get started: just know the examples and get started.
* Rich features: meet most development scenarios

## Examples

# GET request example
```cpp
#include"network/http/http_server.hpp"

using namespace coral;
using json = nlohmann::json;

int main() {

	Router& r = Router::instance();
	r.GET("/", [](Request& req, Response& rsp) {
		rsp.setPath("coarl.json"); //The name doesn't matter, just set it to json format
		json hello = {
			{"msg", "hello!"},
			{"code", 200}
		};
		rsp.write(hello.dump());
	});

	IoContext ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return 0;
}

```
