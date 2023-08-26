//#ifndef TASK_HPP
//#define TASK_HPP
//
//#include<coroutine>
//#include<exception>
//
//namespace coral {
//
//    template<typename T = void>
//    struct task {
//        struct promise_type {
//            auto initial_suspend() noexcept { return std::suspend_never{}; }
//            auto final_suspend() noexcept { return std::suspend_always{}; }
//            auto get_return_object() { return task{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
//            void return_value(T val) { val_ = val; }
//            auto yield_value(T val) {
//                val_ = val;
//                return std::suspend_always{};
//            }
//            void unhandled_exception() {}
//            T val_;
//        };
//        ~task() { handle.destroy(); }
//        T get() { return handle.promise().val_; }
//        bool done() const { return handle.done(); }
//        void resume() { handle.resume(); }
//        auto routine() const { return handle; }
//
//        std::coroutine_handle<promise_type> handle;
//    };
//
//	template<>
//	struct task<void> {
//		struct promise_type {
//			auto initial_suspend() noexcept { return std::suspend_never{}; }
//			auto final_suspend() noexcept { return std::suspend_always{}; }
//			auto get_return_object() { return task{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
//			void return_void() { }
//			void unhandled_exception() {}
//		};
//        ~task() { handle.destroy(); }
//        bool done() const { return handle.done(); }
//		void resume() { handle.resume(); }
//        auto routine() const { return handle; }
//
//		std::coroutine_handle<promise_type> handle;
//	};
//
//} //namespace coral
//
//#endif //TASK_HPP
#ifndef AWAITABLE_HPP
#define AWAITABLE_HPP

//ю╢вт
//https://github.com/franktea/coro_epoll_kqueue/blob/main/task.h

#include <coroutine>
#include <iostream>

using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;

template<typename T> struct awaitable;

namespace detail {

    template<typename T>
    struct promise_type_base {
        coroutine_handle<> continuation_ = std::noop_coroutine(); // who waits on this coroutine
        awaitable<T> get_return_object();
        suspend_always initial_suspend() { return {}; }

        struct final_awaiter {
            bool await_ready() noexcept { return false; }
            void await_resume() noexcept {}

            template<typename promise_type>
            coroutine_handle<> await_suspend(coroutine_handle<promise_type> coro) noexcept {
                return coro.promise().continuation_;
            }
        };

        auto final_suspend() noexcept {
            return final_awaiter{};
        }

        void unhandled_exception() { //TODO: 
            std::exit(-1);
        }
    }; // struct promise_type_base

    template<typename T>
    struct promise_type final : promise_type_base<T> {
        T result;
        void return_value(T value) { result = value; }
        T await_resule() { return result; }
        awaitable<T> get_return_object();
    };

    template<>
    struct promise_type<void> final : promise_type_base<void> {
        void return_void() {}
        void await_resume() {}
        awaitable<void> get_return_object();
    };

} // namespace detail

template<typename T = void>
struct awaitable {
    using promise_type = detail::promise_type<T>;
    awaitable() :handle_(nullptr) {}
    awaitable(coroutine_handle<promise_type> handle) :handle_(handle) {}
    bool await_ready() { return false; }
    T await_resume() {
        return handle_.promise().result;
    }

    void await_suspend(coroutine_handle<> waiter) {
        handle_.promise().continuation_ = waiter;
        handle_.resume();
    }

    void resume() {
        handle_.resume();
    }

    coroutine_handle<promise_type> handle_;
};

namespace detail {
    template<typename T>
    inline awaitable<T> promise_type<T>::get_return_object() {
        return awaitable<T>{ coroutine_handle<promise_type<T>>::from_promise(*this)};
    }

    inline awaitable<void> promise_type<void>::get_return_object() {
        return awaitable<void>{ coroutine_handle<promise_type<void>>::from_promise(*this)};
    }
}

#endif //AWAITABLE_HPP