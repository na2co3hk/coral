#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include<memory>
#include<mutex>
#include<queue>
#include<chrono>
#include<tuple>
#include<thread>
#include<condition_variable>

#include"mysql.hpp"

namespace manjusaka{

template<typename DB>
class connection_pool {
public:
	static connection_pool<DB>& instance() {
		static connection_pool<DB> instance;
		return instance;
	}

	template<typename... Args>
	void init(int maxSize, Args &&...args) {
		std::call_once(flag_, &connection_pool<DB>::template init_impl<Args...>,
			this, maxSize, std::forward<Args>(args)...);
	}

	std::shared_ptr<DB> get() {
		std::unique_lock<std::mutex> lock(mtx_);
		while (conq_.empty()) {
			if (std::cv_status::timeout == cond_.wait_for(lock, std::chrono::seconds(3))) {
				return nullptr; //timeout
			}
		}

		auto con = conq_.front();
		conq_.pop();
		lock.unlock();

		if (con == nullptr or !con->ping()) {
			add();
		}

		return con;
	}

	void produce() {
		while (true) {
			std::unique_lock<std::mutex> lock(mtx_);
			while (conq_.size() >= 3) { //改成minSize
				cond_.wait(lock);
			}
			add();
			cond_.notify_one();
		}
	}

	void recycle() {
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			std::lock_guard<std::mutex> lock(mtx_);
			while (conq_.size() > 3) { //改成minSize
				std::shared_ptr<DB> con = conq_.front();
				if (con->getAliveTime() >= 3000) {
					conq_.pop();
					delete con;
				}
				else {
					break;
				}
			}
		}
	}

private:
	template<typename... Args>
	void init_impl(int maxSize, Args &&...args) {
		args_ = std::make_tuple(std::forward<Args>(args)...);

		for (int i = 0; i < maxSize; i++) {
			auto con = std::make_shared<DB>();
			if (con->connect(std::forward<Args>(args)...)) {
				conq_.push(con);
			}
			else {
				throw std::invalid_argument("init falled");
			}
		}
	}

	auto add() {
		auto con = std::make_shared<DB>();
		auto fn = [con](auto... args) {
			return con->connect(args...);
		};

		return std::apply(fn, args_) ? con : nullptr;
	}

	connection_pool() = default;
	~connection_pool() = default;
	connection_pool(const connection_pool&) = delete;
	connection_pool& operator=(const connection_pool&) = delete;

	std::once_flag flag_;
	std::tuple<const char*, const char*, const char*, const char*>args_;
	std::mutex mtx_;
	std::condition_variable cond_;
	using ConnectionQueue = std::queue<std::shared_ptr<DB>>;
	ConnectionQueue conq_;
};
}

#endif //CONNECTION_POOL_H