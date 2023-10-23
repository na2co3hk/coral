#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>

#include"../../base/annotation.hpp"
#include"../../base/string_handler.hpp";
#include"../../base/basic_buffer.hpp";
#include"../../log/logstream.hpp"

namespace coral {

class Request {
public:
	enum STATE
	{
		REQUEST_LINE,
		HEADERS,
		BODY,
		FINISH,
	};

	enum HTTP_CODE
	{
		NO_REQUEST = 0,
		GET_REQUEST,
		BAD_REQUEST,
		NO_RESOURSE,
		FORBIDDENT_REQUEST,
		FILE_REQUEST,
		INTERNAL_ERROR,
		CLOSED_CONNECTION,
	};

    Request() = default;

	Request(const Buffer& msg):
		err_(0),
		state_(REQUEST_LINE)
	{
		headers_.clear();
		query_.clear();
		parse(msg);
	}

	AUTO_GET_SET(method_, Method);
	AUTO_GET_SET(path_, Path);
	AUTO_GET_SET(version_, Version);
	AUTO_GET_SET(err_, Err);
	AUTO_GET_SET(body_, Body);

    Request& setHeader(const std::string& key, const std::string& val) {
        headers_[key] = val;
        return *this;
    }

	std::unordered_map<std::string, std::string> getHeaders() const {
		return headers_;
	}

	Request& setParams(const std::string key, const std::string val) {
		params_[key] = val;
        return *this;
	}

	std::string getQuery(const std::string& key) const {
		if (query_.find(key) != query_.end()) {
			return query_.find(key)->second;
		}
		return "null";
	}

	std::string getParam(const std::string& key) const {
		if (params_.find(key) != params_.end()) {
			return params_.find(key)->second;
		}
		return "null";
	}

	bool isKeepAlive() const {
		if (headers_.find("Connection") != headers_.end()) {
			return (headers_.find("Connection")->second == "keep-alive") and
				(version_ == "1.1") ? true : false;
		}
		return false;
	}

    std::string serialize() {
        std::string req;
        req += method_ + " " + path_ + " " + "HTTP/" + version_ + " " + "\r\n";
        for(auto [key, value] : headers_) {
            req += key + ": " + value + "\r\n";
        }
        req += "\r\n";
        req += body_;
        return req;
    }

private:

	void parse(const Buffer& msg) {
		const char* CRLF = "\r\n";
		if (msg.readableBytes() <= 0) {
			err_ = 1;
			LOG_ERROR << "No request";
			return;
		}

		std::vector<std::string>lines;
		std::string request(msg.peek(), msg.readableBytes());
		lines = split(request, CRLF);
		for (std::string& line : lines)
		{
			switch (state_) {
			case REQUEST_LINE:
				parseRequestLine(line);
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
		if (body_.empty()) {
			state_ = FINISH;
		}
		if (state_ != FINISH) {
			LOG_ERROR << "Parse error";
			return;
		}
		if (err_) {
			path_ = "/404.html";
			LOG_ERROR << "Parse error";
			return;
		}
	}

	void parseRequestLine(const std::string& line) {
		std::vector<std::string> subMatch;
		subMatch = split(line, " ");
		if (subMatch.size() < 3) {
			err_ = 1;
			LOG_ERROR << "RequestLine parse error";
			return;
		}
		else {
			method_ = subMatch[0];
			
			size_t idx = subMatch[1].find_first_of("?");
			if (idx != std::string::npos) {
				parseParams(subMatch[1].substr(idx + 1));
				path_ = subMatch[1].substr(0, idx);
			}
			else {
				path_ = subMatch[1];
			}
			version_ = split(subMatch[2], "/")[1];

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
			err_ = 1;
			LOG_ERROR << "Headers parse error";
			return;
		}
		else {
			headers_[subMatch[0]] = subMatch[1];
		}
	}

	void parseBody(const std::string& line) {
			
		if (method_ == "GET") {
			state_ = FINISH;
			return;
		}
		
		body_ += line + "\r\n";
        state_ = FINISH;
	}

	void parseParams(const std::string& line) {

		std::vector<std::string> subMatch;
		subMatch = split(line, "&");
		for (std::string match : subMatch) {
			size_t idx = match.find_first_of("=");
			if (idx != std::string::npos) {
				query_[match.substr(0, idx)] = match.substr(idx + 1);
			}
		}
	}

	int err_;
	STATE state_;
	std::string method_, path_, version_;
	std::unordered_map<std::string, std::string>headers_;
	std::unordered_map<std::string, std::string>query_;
	std::unordered_map<std::string, std::string>params_;
	std::string body_;
};

} //namespace coral

#endif //HTTP_REQUEST_HPP
