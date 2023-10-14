#ifndef LOGGER_HPP
#define LOGGER_HPP

#include<string>
#include<string_view>
#include<fstream>
#include<sstream>
#include<vector>
#include<ctime>
#include<time.h>
#include<chrono>
//#include <iomanip> //puttime;

#include"../base/noncopyable.h"
#include"../coroutine/awaitable.hpp"
#include"../coroutine/chan.hpp"

namespace coral {

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

    const char* serverName = LIGHT_CYAN"[CORAL] ";
    const uint32_t chanLength = 1000;  //ͨ����󳤶ȣ����г����ó��ȵ���Ϣ�������
    const uint32_t maxBufferLength = 10 * 1024; //��־�������ﵽ�ô�С��ˢ��
    const uint64_t flushInterval = 1 * 1000; //ˢ�̼����Ĭ��1s

    enum CRLOG_LEVEL : std::uint8_t {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LEVEL_COUNT,
    };

    const std::vector<std::string_view> LEVEL_CHAR = {
            LIGHT_BLUE"[Debug] : ",
            LIGHT_GREEN"[Info] : ",
            YELLOW"[Warn] : ",
            LIGHT_RED"[Error] : ",
            LIGHT_GRAY"[Fatal] : ",
    };

    class LogMessage {
    public:
        LogMessage(const std::string_view msg, std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now()) :
                msg_(msg),
                time_(time) {}

        std::string toString(CRLOG_LEVEL level) {
            std::stringstream ss;
            ss << serverName << LIGHT_PURPLE << "[" << FormatTime() << "] " << LIGHT_GREEN << LEVEL_CHAR[level] << msg_ << NONE << std::endl;
            return ss.str();
        }

    private:

        std::string FormatTime() {
            time_t tv = std::chrono::system_clock::to_time_t(time_);
            char tmp[64];
            strftime(tmp, sizeof(tmp), "%Y-%m-%d %X", localtime(&tv));
            return tmp;
        }

        std::string msg_;
        std::chrono::high_resolution_clock::time_point time_;
    };

    class Logger : noncopyable {
    public:

        Logger() :
                chan_(chanLength) {}

        static Logger& instance() {
            static Logger instance;
            return instance;
        }

        void Open(const std::string_view file) {

        }

        bool batchFlush() {

            return true;
        }

        void Debug(const std::string_view msg) {
            std::cout << LogMessage(msg).toString(CRLOG_LEVEL::DEBUG);
        }

        void Info(const std::string_view msg) {
            std::cout << LogMessage(msg).toString(CRLOG_LEVEL::INFO);
        }

        void Warn(const std::string_view msg) {
            std::cout << LogMessage(msg).toString(CRLOG_LEVEL::WARN);
        }

        void Error(const std::string_view msg) {
            std::cout << LogMessage(msg).toString(CRLOG_LEVEL::ERROR);
        }

        void Fatal(const std::string_view msg) {
            std::cout << LogMessage(msg).toString(CRLOG_LEVEL::FATAL);
        }

    private:
        std::stringstream buf_;
        chan<LogMessage>chan_;
    };

Logger& log = Logger::instance();

} // namespace coral

#endif //LOGGER_HPP
