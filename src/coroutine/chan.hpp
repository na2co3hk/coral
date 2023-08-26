#ifndef CHAN_HPP
#define CHAN_HPP

//模拟golang的协程
//完全来自(一点都没改)
//https://github.com/bennyhuo/CppCoroutines/blob/master/07.channel/Channel.h
//不过也发现了一点缺点，无法在非协程的环境下使用

#include<coroutine>
#include<stop_token>
#include<condition_variable>
#include<mutex>
#include<queue>
#include<list>
#include<exception>
#include<iostream>
#include<functional>

namespace coral {

//仿go风格的一个make
#define make(channel, type, capacity) coral::channel<type>(capacity)

template<typename ValueType>
struct chan;

template<typename ValueType>
struct WriterAwaiter {
    chan<ValueType>* channel;
    ValueType _value;
    std::coroutine_handle<> handle;

    WriterAwaiter(chan<ValueType>* channel, ValueType value)
        : channel(channel), _value(value) {}

    WriterAwaiter(WriterAwaiter&& other) noexcept
        : channel(std::exchange(other.channel, nullptr)),
        _value(other._value),
        handle(other.handle) {}


    bool await_ready() {
        return false;
    }

    auto await_suspend(std::coroutine_handle<> coroutine_handle) {
        this->handle = coroutine_handle;
        channel->try_push_writer(this);
    }

    void await_resume() {
        channel->check_closed();
        channel = nullptr;
    }

    void resume() {
        handle.resume();
    }

    ~WriterAwaiter() {
        if (channel) channel->remove_writer(this);
    }
};

template<typename ValueType>
struct ReaderAwaiter {
    chan<ValueType>* channel;
    ValueType _value;
    ValueType* p_value = nullptr;
    std::coroutine_handle<> handle;

    explicit ReaderAwaiter(chan<ValueType>* channel) : channel(channel) {}

    ReaderAwaiter(ReaderAwaiter&& other) noexcept
        : channel(std::exchange(other.channel, nullptr)),
        _value(other._value),
        p_value(std::exchange(other.p_value, nullptr)),
        handle(other.handle) {}

    bool await_ready() { return false; }

    auto await_suspend(std::coroutine_handle<> coroutine_handle) {
        this->handle = coroutine_handle;
        channel->try_push_reader(this);
    }

    auto await_resume() {
        auto channel = this->channel;
        this->channel = nullptr;
        channel->check_closed();
        return _value;
    }

    void resume(ValueType value) {
        this->_value = value;
        if (p_value) {
            *p_value = value;
        }
        resume();
    }

    void resume() {
        handle.resume();
    }

    ~ReaderAwaiter() {
        if (channel) channel->remove_reader(this);
    }
};

template<typename ValueType>
struct chan {

    struct ChannelClosedException : std::exception {
        const char* what() const noexcept override {
            return "Channel is closed.";
        }
    };

    void check_closed() {
        if (!_is_active.load(std::memory_order_relaxed)) {
            throw ChannelClosedException();
        }
    }

    void try_push_reader(ReaderAwaiter<ValueType>* reader_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();

        if (!buffer.empty()) {
            auto value = buffer.front();
            buffer.pop();

            if (!writer_list.empty()) {
                auto writer = writer_list.front();
                writer_list.pop_front();
                buffer.push(writer->_value);
                lock.unlock();

                writer->resume();
            }
            else {
                lock.unlock();
            }

            reader_awaiter->resume(value);
            return;
        }

        if (!writer_list.empty()) {
            auto writer = writer_list.front();
            writer_list.pop_front();
            lock.unlock();

            reader_awaiter->resume(writer->_value);
            writer->resume();
            return;
        }

        reader_list.push_back(reader_awaiter);
    }

    void try_push_writer(WriterAwaiter<ValueType>* writer_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();
        // suspended readers
        if (!reader_list.empty()) {
            auto reader = reader_list.front();
            reader_list.pop_front();
            lock.unlock();

            reader->resume(writer_awaiter->_value);
            writer_awaiter->resume();
            return;
        }

        // write to buffer
        if (buffer.size() < buffer_capacity) {
            buffer.push(writer_awaiter->_value);
            lock.unlock();
            writer_awaiter->resume();
            return;
        }

        // suspend writer
        writer_list.push_back(writer_awaiter);
    }

    void remove_writer(WriterAwaiter<ValueType>* writer_awaiter) {
        std::lock_guard lock(channel_lock);
        auto size = writer_list.remove(writer_awaiter);
    }

    void remove_reader(ReaderAwaiter<ValueType>* reader_awaiter) {
        std::lock_guard lock(channel_lock);
        auto size = reader_list.remove(reader_awaiter);
    }

    auto write(ValueType value) {
        check_closed();
        return WriterAwaiter<ValueType>(this, value);
    }

    auto operator<<(ValueType value) {
        return write(value);
    }

    auto read() {
        check_closed();
        return ReaderAwaiter<ValueType>(this);
    }

    auto operator>>(ValueType& value_ref) {
        auto awaiter = read();
        awaiter.p_value = &value_ref;
        return awaiter;
    }

    void close() {
        bool expect = true;
        if (_is_active.compare_exchange_strong(expect, false, std::memory_order_relaxed)) {
            clean_up();
        }
    }

    explicit chan(int capacity = 0) : buffer_capacity(capacity) {
        _is_active.store(true, std::memory_order_relaxed);
    }

    bool is_active() {
        return _is_active.load(std::memory_order_relaxed);
    }

    chan(chan&& channel) = delete;

    chan(chan&) = delete;

    chan& operator=(chan&) = delete;

    ~chan() {
        close();
    }

private:
    int buffer_capacity;
    std::queue<ValueType> buffer;
    std::list<WriterAwaiter<ValueType>*> writer_list;
    std::list<ReaderAwaiter<ValueType>*> reader_list;

    std::atomic<bool> _is_active;

    std::mutex channel_lock;
    std::condition_variable channel_condition;

    void clean_up() {
        std::lock_guard lock(channel_lock);

        for (auto writer : writer_list) {
            writer->resume();
        }
        writer_list.clear();

        for (auto reader : reader_list) {
            reader->resume();
        }
        reader_list.clear();

        decltype(buffer) empty_buffer;
        std::swap(buffer, empty_buffer);
    }
};

} //namespace coral

#endif //CHAN_HPP