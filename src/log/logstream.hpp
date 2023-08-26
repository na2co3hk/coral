#ifndef LOG_STREAM_Hpp
#define LOG_STREAM_Hpp

#include<type_traits>
#include<chrono>
#include<string>
#include<vector>
#include<functional>
#include<iostream>

#include"../base/noncopyable.h"
#include"../base/basic_buffer.hpp"

#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"

enum LOG_LEVEL { 
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL,
	LEVEL_COUNT,
};

std::string getLevelName[LOG_LEVEL::LEVEL_COUNT]{
	CYAN"[DEBUG]",
	GREEN"[INFO]",
	BROWN"[WARN]",
	RED"[ERROR]",
	LIGHT_PURPLE"[FATAL]",
};

class LogStream : noncopyable {

public:
	LogStream(const char* file, int line, LOG_LEVEL level = LOG_LEVEL::INFO) :
		fileName_(file), 
		line_(line), 
		level_(level),
		buf_()
	{
		auto now = std::chrono::system_clock::now();
		auto t = std::chrono::system_clock::to_time_t(now);
		std::string cur(ctime(&t));
		cur.erase(cur.end() - 1, cur.end());
		buf_ += DARY_GRAY"[MANJUSAKA]";
		buf_ += "[==";
		buf_ += cur;
		buf_ += "==]";
		buf_ += getLevelName[level];
		buf_ += ": ";

	}

	~LogStream() {
		
		buf_ += " -";
		buf_ += fileName_;
		buf_ += ":";
		buf_ += std::to_string(line_);
		buf_ += NONE;
		buf_ += "\n";
		output_(buf_.peek(), buf_.readableBytes());
		if (level_ == LOG_LEVEL::FATAL) {
			flush_();
			abort();
		}
	}

	LogStream& operator<<(const std::string& str) {
		buf_ += str;
		return *this;
	}

	LogStream& operator<<(const char* str) {
		buf_.append(str, strlen(str));
		return *this;
	}

	template<typename T>
	LogStream& operator<<(const T& t) {
		using U = std::remove_const_t<std::remove_reference_t<T>>;
		if constexpr (std::is_arithmetic_v<U>) {
			std::string str = std::to_string(t);
			buf_ += str;
		}
		return *this;
	}

	LOG_LEVEL level() {
		return level_;
	}

	using OutPutFunc = std::function<void(const char* data, size_t len)>;
	using FlushFunc = std::function<void()>;

	static void defaultOutputFunc(const char* data, size_t len) {
		fwrite(data, len, sizeof(char), stdout);
	}

	static void defaultFlush() {
		fflush(stdout);
	}

	OutPutFunc output_ = defaultOutputFunc;
	FlushFunc flush_ = defaultFlush;

	void setLogLevel(LOG_LEVEL level) { level_ = level; }
	void setOutput(OutPutFunc func) { output_ = func; }
	void setFlush(FlushFunc func) { flush_ = func; }

private:
	LOG_LEVEL level_;
	const char* fileName_;
	int line_;
	coral::Buffer buf_;
};

#define LOG_INFO LogStream(__FILE__, __LINE__, LOG_LEVEL::INFO)
#define LOG_WARN LogStream(__FILE__, __LINE__, LOG_LEVEL::WARN)
#define LOG_ERROR LogStream(__FILE__, __LINE__, LOG_LEVEL::ERROR)
#define LOG_FATAL LogStream(__FILE__, __LINE__, LOG_LEVEL::FATAL)

#endif //LOG_STREAM_HPP
