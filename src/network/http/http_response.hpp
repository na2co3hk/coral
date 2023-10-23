#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h> 
#include<string>
#include<fstream>
#include<unordered_map>

#include"cookie.hpp"
#include"http_info.hpp"
#include"../../log/logstream.hpp"
#include"../../base/basic_buffer.hpp"
#include"../../base/string_handler.hpp"

namespace coral {

class Response {
public:

    enum STATE
    {
        STATE_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    Response(Buffer& buf, int code = 0, bool isKeepAlive = false) :
        buf_(buf),
        code_(code),
        isKeepAlive_(isKeepAlive)
    {
        buf_.retrieveAll();
        headers_.clear();
    }

//    Response(Buffer& buf, bool deserialize = false):
//        buf_(buf),
//        state_(STATE_LINE)
//    {
//        headers_.clear();
//        parse(buf);
//    }

    Response(Buffer& buf, bool deserialize = false):
       buf_(buf) {};

    void deserialize() {
        state_ = STATE_LINE;
        headers_.clear();
        parse(buf_);
    }

    void parse(const Buffer& msg) {
        const char* CRLF = "\r\n";
        if (msg.readableBytes() <= 0) {
            code_ = 404;
            LOG_ERROR << "No response";
            return;
        }

        std::vector<std::string>lines;
        std::string response(msg.peek(), msg.readableBytes());
        lines = split(response, CRLF);
        for (std::string& line : lines) {
            switch (state_) {
            case STATE_LINE:
                parseStateLine(line);
                break;
            case HEADERS:
                parseHeader(line);
                break;
            case BODY:
                parseBody(line);
                break;
            default:
                break;
            }
        }

        if (!body_.empty()) {
            state_ = FINISH;
        }

        if (state_ != FINISH) {
            LOG_ERROR << "Parse error";
            return;
        }
        if (code_ == 404) {
            LOG_ERROR << "Parse error";
            return;
        }
    }

    void addStateLine() {

        if (path_ != "") {
            if (stat((path_).data(), &fileState_) < 0 or S_ISDIR(fileState_.st_mode) or code_ == 1) {
                code_ = 404;
            }
            else if (!(fileState_.st_mode & S_IROTH)) { // û��Ȩ��
                code_ = 403;
            }
            else if (code_ == 0) {
                code_ = 200;
            }
        }
        if (body_ != "") {
            code_ = 200;
        }

        std::string status;
        if (CODE_STATUS.count(code_) == 1) {
            status = CODE_STATUS.find(code_)->second;
        }
        else {
            code_ = 400;
            status = CODE_STATUS.find(code_)->second; //BAD REQUEST
        }
        buf_.append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
    }

    //����Ӧͷд��������
    void addHeader() {

        for (auto header : headers_) {
            buf_.append(header.first + ": " + header.second);
            buf_.append("\r\n");
        }

        buf_.append("Connection: ");
        if (isKeepAlive_)
        {
            buf_.append("keep-alive\r\n");
            buf_.append("keep-alive: max=6, timeout=120\r\n");
        }
        else
        {
            buf_.append("close\r\n");
        }

        buf_.append("Content-type: " + getType() + "; charset=UTF-8" + "\r\n\r\n");
    }

    void addBody() {
        buf_.append(body_);
    }

    std::string getHeader(const std::string& key) const {

        if (headers_.count(key) == 1) {
            return headers_.find(key)->second;
        }
        else {
            LOG_ERROR << "No header";
            return "";
        }
    }

    //�����Ӧͷ
    void setHeader(const std::string& key, const std::string& val) {
        headers_[key] = val;
    }

    std::string getType() {

        std::size_t idx = path_.find_last_of('.');
        if (idx == std::string::npos)
        {
            return "text/plain";
        }
        std::string suffix = path_.substr(idx);
        if (SUFFIX_TYPE.count(suffix) == 1)
        {
            return SUFFIX_TYPE.find(suffix)->second;
        }
        return "text/plain";
    }

    void setPath(const std::string& path) {
        path_ = path;
    }

    std::string getPath() const {
        return path_;
    }

    void setCode(int code) {
        code_ = code;
    }

    void write(const std::string& msg) {
        body_ += msg;
    }

    void json(const std::string& msg) {
        setPath(".json");
        body_ += msg;
    }

    std::string getBody() const {
        return body_;
    }

    void setCookie(Cookie& cookie) {
        setHeader("Set-Cookie", cookie.to_string());
    }

private:

    void parseStateLine(const std::string& line) {
        std::vector<std::string> subMatch;
        subMatch = split(line, " ");
        if (subMatch.size() < 3) {
            code_ = 404;
            LOG_ERROR << "StateLine parse error";
            return;
        }
        else {
            version_ = split(subMatch[0], "/")[1];
            code_ = atoi(subMatch[1].data());
        }
        state_ = HEADERS;
        return;
    }

    void parseHeader(const std::string& line) {
        if (line == "") {
            state_ = BODY;
            return;
        }

        std::vector<std::string> subMatch;
        subMatch = split(line, ": ");
        if (subMatch.size() < 2) {
            code_ = 404;
            LOG_ERROR << "Headers parse error";
            return;
        }
        else {
            headers_[subMatch[0]] = subMatch[1];
        }
    }

    void parseBody(const std::string& line) {
        body_ += line + "\r\n";
    }

    std::string body_{};
    std::string path_{};
    Buffer& buf_;
    struct stat fileState_; //�ļ�״̬
    int code_;
    bool isKeepAlive_;
    std::unordered_map<std::string, std::string>headers_;

    std::string version_;
    STATE state_;
};

} //namespace coral

#endif //HTTP_RESPONSE_HPP