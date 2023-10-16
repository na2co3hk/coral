#ifndef LOOP_THREAD_POOL_HPP
#define LOOP_THREAD_POOL_HPP

#include<vector>
#include<memory>
#include<thread>

#include"io_context.h"
#include"../base/noncopyable.h"

namespace coral {

class LoopThreadPool : noncopyable {
public:
	LoopThreadPool(size_t nums = 0):
		nums_(nums)
	{
		for (size_t i = 0; i < nums; i++) {
			loops_.push_back(new Context);
		}
	}

	~LoopThreadPool() {
		for (auto loop : loops_) {
			delete loop;
		}
	}

	void start() {
		std::vector<std::shared_ptr<std::thread>> threads;

		// �����̣߳������¼�ѭ��
		for (std::size_t i = 0; i < nums_; ++i) {
			threads.push_back(std::make_shared<std::thread>([this, i] {
				loops_[i]->run();
			}));
		}

		// �ȴ��߳̽���
		for (auto& thread : threads)
			thread->detach();
	}

	Context& getNextLoop() {
		Context& loop = *loops_[index_];
		index_ = (index_ + 1) % nums_;
		return loop;
	}

private:
	size_t nums_;
	size_t index_{ 0 };
	std::vector<Context*> loops_;
};

} //namespace coral

#endif //LOOP_THREAD_POOL_HPP