#ifndef INET_ADDRESS_HPP
#define INET_ADDRESS_HPP

#include<arpa/inet.h>
#include<sys/socket.h>
#include<string>
#include<string.h>

namespace coral {

class InetAddress {
public:
	explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1") {
		bzero(&addr_, sizeof(addr_));
		addr_.sin_family = AF_INET;
		addr_.sin_port = htons(port);
		addr_.sin_addr.s_addr = INADDR_ANY;
	}
	explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}

	std::string toIp() const {
		char buf[64] = { 0 };
		inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
		return buf;
	}

	uint16_t toPort() const {
		return ntohs(addr_.sin_port);
	}

	std::string toIpPort() const //用于标识一个TCP连接
	{
		char buf[64] = { 0 };
		::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
		size_t end = ::strlen(buf);
		uint16_t port = ::ntohs(addr_.sin_port);
		sprintf(buf + end, ":%u", port);
		return buf;
	}

	const sockaddr_in* get() const { return &addr_; }
	void set(const sockaddr_in& addr) { addr_ = addr; }

private:
	sockaddr_in addr_;
};

} //namespace coral

#endif //INET_ADDRESS_HPP

