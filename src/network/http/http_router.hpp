#ifndef HTTP_ROUTER_HPP
#define HTTP_ROUTER_HPP

#include<string>
#include<string_view>
#include<set>
#include<functional>
#include<memory>
#include<queue>
#include<regex>

#include"http_request.hpp"
#include"http_response.hpp"
#include"../../base/aop.hpp"

namespace coral {

//模仿gin框架自己实现的一个基数树，优点：节省内存？
//优化：用泛型和枚举类合并多个基数树
class RadixTreeNode {
public:
	using Handler = std::function<void(Request&, Response&)>;

	explicit RadixTreeNode(const std::string& key, const Handler& handler) :
		key_(key),
		priority_(0),
		handler_(handler) {}

	bool operator<(const std::shared_ptr<RadixTreeNode>& other) {
		return this->priority_ < other->priority_;
	}

	std::string key_;
	Handler handler_;
	size_t priority_;
	std::set<std::shared_ptr<RadixTreeNode>>children_;

};


class RadixTree {
public:
	using Handler = std::function<void(Request&, Response&)>;
	using RadixTreeNodePtr = std::shared_ptr<RadixTreeNode>;

	RadixTree() :
		root_(new RadixTreeNode("/", Handler())) { }

	~RadixTree() {}

	void insert(const std::string& key, const Handler& handler) {
		if (key == "/") {
			root_->handler_ = handler;
		}
		else {
			insertImpl(key.substr(1, key.size()), handler, root_);
		}
	}

	void erase(const std::string& key) {
		if (key == "/") {
			root_->handler_ = Handler();
		}
		else {
			eraseImpl(key.substr(1, key.size()), root_);
		}
	}

	Handler search(const std::string& key) {
		if (key == "/") {
			return root_->handler_;
		}
		return searchImpl(key.substr(1, key.size()), root_);
	}

	void bfs() {
		std::queue<RadixTreeNodePtr>q;
		q.push(root_);

		while (!q.empty()) {
			int cnt = q.size();
			for (int i = 0; i < cnt; ++i) {
				auto cur = q.front();
				q.pop();
				std::cout << cur->key_ << " ";
				for (auto child : cur->children_) {
					q.push(child);//子节点入队
				}
			}
			std::cout << std::endl;
		}
	}

private:

	std::shared_ptr<RadixTreeNode> root_;

	void insertImpl(const std::string& key, const Handler& handler, RadixTreeNodePtr node) {

		//没有子节点，直接插入
		if (node->children_.empty()) {
			auto newNode = std::make_shared<RadixTreeNode>(key, handler);
			node->children_.insert(newNode);
			return;
		}

		bool isMatch = false;
		for (auto cur : node->children_) {
			int i = 0;
			for (; i < key.size() and cur->key_.size(); i++) {
				if (key[i] != cur->key_[i]) {
					break;
				}
			}

			if (i != 0) {
				isMatch = true;
				//1：节点的字符串与待插入字符串完全匹配，相当于修改
				if (i == key.size() and i == cur->key_.size()) {
					cur->handler_ = handler;
				}
				else if (i != cur->key_.size()) {
					//2: 节点是待插入字符串的前缀
					auto newNode = std::make_shared<RadixTreeNode>(cur->key_.substr(i), cur->handler_);

					cur->key_ = cur->key_.substr(0, i);
					cur->handler_ = (i == key.size()) ? handler : Handler();
					cur->children_.swap(newNode->children_);
					cur->children_.insert(newNode);

					if (i != key.size()) {
						auto newNode2 = std::make_shared<RadixTreeNode>(key.substr(i), handler);
						cur->children_.insert(newNode2);
					}
				}
				else {
					//继续递归匹配
					insertImpl(key.substr(i), handler, cur);
				}

				if (isMatch) {
					return;
				}
			}
		}

		auto newNode = std::make_shared<RadixTreeNode>(key, handler);
		node->children_.insert(newNode);
	}

	RadixTreeNodePtr eraseImpl(const std::string& key, RadixTreeNodePtr node) {

		bool isMatch = false;

		for (auto cur : node->children_) {
			int i = 0;
			for (; i < key.size() and i < cur->key_.size(); i++) {
				if (key[i] != cur->key_[i]) {
					break;
				}
			}

			if (i != 0) {
				isMatch = true;
				//1：节点的字符串与待插入字符串完全匹配
				if (i == key.size() and i == cur->key_.size()) {
					if (cur->children_.empty()) {
						node->children_.erase(cur);
					}
					else {
						cur->handler_ = Handler();
					}

					//删除该节点后父亲节点只有一个子节点了，直接合并
					if (node->children_.size() == 1 and !node->handler_ and node != root_) {
						auto subNode = *node->children_.begin();
						node->children_.erase(subNode);
						node->handler_ = subNode->handler_;
						node->key_.append(subNode->key_);
						node->children_ = subNode->children_;
						return node;
					}
				}
				else if (i == cur->key_.size()) {
					//2: 当前节点为字符串的前缀
					auto subNode = eraseImpl(key.substr(i), cur);
					if (subNode and node->children_.size() == 1 and !node->handler_
						and node != root_) {
						auto subNode = *node->children_.begin();
						node->children_.erase(subNode);
						node->handler_ = subNode->handler_;
						node->key_.append(subNode->key_);
						node->children_ = subNode->children_;
					}
				}
				else {
					//3: 字符串是当前节点的前缀，节点不存在
					break;
				}
			}
			if (isMatch) {
				return nullptr;
			}
		}
		return nullptr;
	}

	Handler searchImpl(const std::string& key, RadixTreeNodePtr node) {

		for (auto cur : node->children_) {
			int i = 0;
			for (; i < key.size() and i < cur->key_.size(); i++) {
				if (key[i] != cur->key_[i]) {
					break;
				}
			}

			if (i != 0) {
				if (i == key.size() and i == cur->key_.size()) {
					return cur->handler_;
				}
				else if (i == cur->key_.size()) {
					return searchImpl(key.substr(i), cur);
				}
				else {
					return Handler();
				}
			}
		}
		return Handler();
	}

};

class Router : noncopyable {
public:
	using Handler = std::function<void(Request&, Response&)>;

	static Router& instance() {
		static Router instance;
		return instance;
	}
	
	void handleHTTPRequest(Request& req, Response& rsp) {

		const std::string path = req.getPath();
		const std::string method = req.getMethod();
		const std::string version = req.getVersion();

		const std::string key = "@" + method + ":" + path;

		Handler handler = httpStorage_.search(key);
		if (handler) {
			handler(req, rsp);
		}
		else if (isRestfulUrl(req, rsp)) {
			return;
		}
		else {
			rsp.setCode(404);
			std::string NotFoundPage = "/404.html";
			rsp.setPath(NotFoundPage);
			//TODO：发送404页面
		}
	}

	/*void GET(const std::string& path, Handler handler) {
		httpStorage_.insert(path, std::move(handler));
	}*/

	/*void POST(const std::string& path, Handler handler) {
		postHandler_.insert(path, std::move(handler));
	}*/

	/*template<typename... Args>
	void GET(const std::string& path, Handler handler, Args&&... args) {
		Handler newHandler = [&](Request& req, Response& rsp) {
			coral::Invoke<Args...>(handler, req, rsp);
			};
		httpStorage_.insert(path, std::move(newHandler));
	}

	template<typename... Args>
	void POST(const std::string& path, Handler handler, Args&&... args) {
		Handler newHandler = [&](Request& req, Response& rsp) {
			coral::Invoke<Args...>(handler, req, rsp);
			};
		postHandler_.insert(path, std::move(newHandler));
	}*/

	void GET(const std::string& path, Handler handler) {

		const std::string newPath = "@GET:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(handler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(handler));
		}

	}

	void POST(const std::string& path, Handler handler) {
		const std::string newPath = "@POST:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(handler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(handler));
		}
	}

	void PUT(const std::string& path, Handler handler) {
		const std::string newPath = "@PUT:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(handler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(handler));
		}
	}

	void DELETE(const std::string& path, Handler handler) {
		const std::string newPath = "@DELETE:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(handler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(handler));
		}
	}

	template<typename... Args>
	void GET(const std::string& path, Handler handler, Args&&... args) {
		Handler newHandler = [handler](Request& req, Response& rsp) { //引用捕获有bug,会捕获到其他请求方式的handler	
			coral::Invoke<Args...>(handler, req, rsp);
		};

		const std::string newPath = "@GET:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(newHandler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(newHandler));
		}
	}

	template<typename... Args>
	void POST(const std::string& path, Handler handler, Args&&... args) {
		Handler newHandler = [handler](Request& req, Response& rsp) {
			coral::Invoke<Args...>(handler, req, rsp);
		};

		const std::string newPath = "@POST:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(newHandler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(newHandler));
		}
	}

	template<typename... Args>
	void PUT(const std::string& path, Handler handler, Args&&... args) {
		Handler newHandler = [handler](Request& req, Response& rsp) {
			coral::Invoke<Args...>(handler, req, rsp);
			};

		const std::string newPath = "@PUT:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(newHandler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(newHandler));
		}
	}

	template<typename... Args>
	void DELETE(const std::string& path, Handler handler, Args&&... args) {
		Handler newHandler = [handler](Request& req, Response& rsp) {
			coral::Invoke<Args...>(handler, req, rsp);
			};

		const std::string newPath = "@DELETE:" + path;

		if (path.find("{:") != std::string::npos) {
			std::string pattern;
			std::unordered_map<std::string, int> params;
			getRegexPattern(newPath, pattern, params);
			restfulPattern_.insert(pattern);
			restfulStorage_[pattern] = { std::move(params), std::move(newHandler) };
		}
		else {
			httpStorage_.insert(newPath, std::move(newHandler));
		}
	}

private:

	bool isRestfulUrl(Request& req, Response& rsp) {
		std::smatch match;
		const std::string path = "@" + req.getMethod() + ":" + req.getPath();
		std::string key;
		for (auto pattern : restfulPattern_) {
			if (std::regex_match(path, match, std::regex(pattern))) {
				key = pattern;
			}
		}

		if (restfulStorage_.find(key) == restfulStorage_.end()) {
			return false;
		}

		auto res = restfulStorage_[key];
		for (auto [param, idx] : res.first) {
			req.setParams(param, match[idx].str());
		}

		res.second(req, rsp);

		return true;
	}

	bool getRegexPattern(std::string raw, std::string& pattern, std::unordered_map<std::string, int>& params) {
		int count = 0;
		while (raw.find("{:") != std::string::npos) {
			unsigned first = raw.find("{:");
			unsigned last = raw.find("}");
			pattern += raw.substr(0, first);
			pattern += "([^/]*)";
			std::string param = raw.substr(first + 2, last - first - 2);
			if (param.empty()) {
				return false;
			}
			raw = raw.substr(last + 1);
			params[param] = ++count;
			if (count >= 10) {
				return false;
			}
		}
		return true;
	}

	RadixTree httpStorage_;
	std::set<std::string>restfulPattern_;
	std::unordered_map<std::string, std::pair<std::unordered_map<std::string, int>, Handler>>restfulStorage_;
};

} //namespace coral

#endif //HTTP_ROUTER_HPP