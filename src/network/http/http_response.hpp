#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h> 
#include<string>
#include<fstream>
#include<unordered_map>

#include"http_info.hpp"
#include"../../log/logstream.hpp"
#include"../../base/basic_buffer.hpp"

namespace coral {

class Response {
public:
    Response(Buffer& buf, int code = 0, bool isKeepAlive = false) :
        buf_(buf),
        code_(code),
        isKeepAlive_(isKeepAlive)
    {
        buf_.retrieveAll();
        headers_.clear();
    }

    void addStateLine() {

        if (path_ != "") {
            if (stat((path_).data(), &fileState_) < 0 or S_ISDIR(fileState_.st_mode) or code_ == 1) {
                code_ = 404;
            }
            else if (!(fileState_.st_mode & S_IROTH)) { // 没有权限
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

    //将响应头写进缓冲区
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

    //添加响应头
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

    std::string getPath() const {
        return path_;
    }

    std::string getBody() const {
        return body_;
    }

    void setPath(const std::string& path) {
        path_ = path;
    }

    void write(const std::string& msg) {
        body_ += msg;
    }

private:

    std::string body_{};
    std::string path_{};
    Buffer& buf_;
    struct stat fileState_; //文件状态
    int code_;
    bool isKeepAlive_;
    std::unordered_map<std::string, std::string>headers_;
};

} //namespace coral

#endif //HTTP_RESPONSE_HPP