# Coral - Modern C++ HTTP server, based on C++20 coroutine

## About Coral
`coral` is a high-performance modern HTTP server that supports GET/POST requests and database interactions, file uploads (not yet developed), and more.

(Note: It is still under development and some features are not yet perfect)

Here are its main features
* `Excellent performance`: based on C++20 coroutine, instead of the original callback mode, can perform asynchronous operations in a synchronous manner.
* `Easy to get started`: follow the example to get started.
* `Rich features`: meet most development scenarios

`coral` is mainly composed of the following parts
* `base`: It provides some basic data structures, algorithms and common functions.
* `coroutine`: Implement asynchronous system calls using coroutine.
* `log`: Records the status of the running server.
* `network`: Basic network services(TCP/HTTP/UDP).
* `test`: Test framework.

## How to install?
The CMake is supported to build this project, so the first step, let's install CMake first
```shell
sudo apt install cmake
```

then we can build the project by following instructions
```shell
git clone https://github.com/na2co3hk/coral.git
mkdir build
cd build
cmake ..
make 
```

finally, add it into your `CMakeLists.txt` 
```shell
target_link_libraries(yourproject PUBLIC Manjusaka)
```

## Examples

### Simple GET request example
```cpp
#include<coral.h>

using namespace coral;
using json = nlohmann::json;

int main() {

	Router& r = Router::instance();
	r.GET("/", [](Request& req, Response& rsp) {
		rsp.setPath("coarl.json"); //The name doesn't matter, just for json format
		json hello = {
			{"msg", "hello!"},
			{"code", 200}
		};
		rsp.write(hello.dump());
	});

	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return 0;
}

```

### GET request wih arguments example
```cpp
#include<coral.h>

using namespace coral;
using json = nlohmann::json;

int main() {

	Router& r = Router::instance();
	r.GET("/args", [](Request& req, Response& rsp) {
		rsp.setPath("coarl.json");
		std::string name = req.getQuery("name");
		std::string age = req.getQuery("age");
		json hello = {
			{"msg", "hello!"},
			{"code", 200},
			{"name", name},
			{"age", age}
		};
	
		rsp.write(hello.dump());
	});
	
	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return 0;
}
```

### AOP example
```cpp
#include<coral.h>

using namespace coral;
using json = nlohmann::json;

struct LogAspect {

	bool Before(Request& req, Response& rsp) {
		rsp.write("before aspect\r\n");
                return true;
	}

	void After(Request& req, Response& rsp) {
		rsp.write("after aspect\r\n");
	}
};

int main() {

	Router& r = Router::instance();
	r.GET("/aspect", [](Request& req, Response& rsp) {
		rsp.setPath("coral.txt");
		rsp.write("this is AOP test\r\n");
	}, LogAspect{});

	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return RUN_ALL_TESTS();
}
```

### Start a test
```cpp
#include<coral.h>

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
	ASSERT_TRUE(skl.erase(1));
	ASSERT_FALSE(skl.search(1));
}

int main() {
    RUN_ALL_TESTS(); //run all test cases
}
```

### RESTful api example
```cpp
#include<coral.h>

using namespace coral;
using json = nlohmann::json;

int main() {

	Router& r = Router::instance();

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
	});

	Context ctx;
	HTTPServer server("5132", ctx);
	server.run();
	ctx.run();
	return 0;
}
```
## Future plan
- timer - [finish]
- cache
- websocket
- UDP(KCP)
- RPC 

## Reference
* [`cinatra`](https://github.com/qicosmos/cinatra/tree/27721cb849e03f95b271db19d0126240c7de04c4) : modern c++(c++20), cross-platform, header-only, easy to use http framework.
* [`CoNet`](https://github.com/oxc-v/CoNet/tree/main): 一个基于 C++20 的异步协程网络库.
* [`coro_epoll_kqueue`](https://github.com/franktea/coro_epoll_kqueue/tree/main): c++20 coroutine with epoll and queue.
* [`CppCoroutines`](CppCoroutines): C++20 couroutine tutorials.
