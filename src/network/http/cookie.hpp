#ifndef COOKIE_HPP
#define COOKIE_HPP

#include<string>
#include<string.h>
#include<chrono>
#include<time.h>

namespace coral {

	class Cookie {
	public:

		Cookie() = default;
		Cookie(const std::string& name, const std::string& value) :
			name_(name),
			value_(value) { }

		void setName(const std::string& name) {
			name_ = name;
		}

		std::string getName() const {
			return name_;
		}

		void setValue(const std::string& value) {
			value_ = value;
		}

		std::string getValue() const {
			return value_;
		}

		void setDomain(const std::string& domain) {
			domain_ = domain;
		}

		void setPath(const std::string& path) {
			path_ = path;
		}

		void setMaxAge(std::uint32_t seconds) {
			max_age_ = seconds;
		}

		void setHttpOnly(bool http_only) {
			http_only_ = http_only;
		}

		void setSecure(bool secure) {
			secure_ = secure;
		}

		void setVersion(int version) {
			version_ = version;
		}

		void setPriority(const std::string& priority) {
			priority_ = priority;
		}

		void setComment(const std::string& comment) {
			comment_ = comment;
		}

		std::string to_string() {
			std::string result;
			result.reserve(256);
			result.append(name_);
			result.append("=");
			if (version_ == 0) {
				// Netscape cookie
				result.append(value_);
				if (!path_.empty()) {
					result.append("; path=");
					result.append(path_);
				}
				if (!priority_.empty()) {
					result.append("; Priority=");
					result.append(priority_);
				}
				if (max_age_ != -1) {
					result.append("; expires=");
					result.append(GMT());
				}
				if (secure_) {
					result.append("; secure");
				}
				if (http_only_) {
					result.append("; HttpOnly");
				}
			}
			else {
				// RFC 2109 cookie
				result.append("\"");
				result.append(value_);
				result.append("\"");
				if (!comment_.empty()) {
					result.append("; Comment=\"");
					result.append(comment_);
					result.append("\"");
				}
				if (!path_.empty()) {
					result.append("; Path=\"");
					result.append(path_);
					result.append("\"");
				}
				if (!priority_.empty()) {
					result.append("; Priority=\"");
					result.append(priority_);
					result.append("\"");
				}

				if (max_age_ != -1) {
					result.append("; Max-Age=\"");
					result.append(std::to_string(max_age_));
					result.append("\"");
				}
				if (secure_) {
					result.append("; secure");
				}
				if (http_only_) {
					result.append("; HttpOnly");
				}
				result.append("; Version=\"1\"");
			}
			return result;
		}

	private:

		std::string GMT() {

			std::chrono::seconds expires(max_age_);
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			std::chrono::system_clock::time_point newTime = now + expires;

			std::time_t t = std::chrono::system_clock::to_time_t(newTime);
			struct tm* tm = gmtime(&t);
			char buf[29];
			strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S", tm);
			memcpy(buf + 25, " GMT", 5);
			return std::string(buf);
		}

		std::string name_;
		std::string value_;
		std::string domain_;
		std::string path_;
	    std::uint32_t max_age_ = -1;
		bool http_only_ = false;
		bool secure_ = false;

		int version_ = 0;
		std::string priority_;
		std::string comment_;
	};

} //namespace coral

#endif //COOKIE_HPP