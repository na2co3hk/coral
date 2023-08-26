#ifndef BASE_BUFFER_HPP
#define BASE_BUFFER_HPP

#include<vector>
#include<string>
#include<string.h>
#include<sys/uio.h>
#include<unistd.h>

namespace coral {

class Buffer {
public:
	static const size_t PREPEND = 8;
	static const size_t INITSIZE = 1024;

	explicit Buffer(size_t size = INITSIZE) :
		buf_(PREPEND + size),
		reader_(PREPEND),
		writer_(PREPEND) {}

	char* peek() { return begin() + reader_; }
	const char* peek() const { return begin() + reader_; }
	char* beginWrite() { return begin() + writer_; }
	const char* beginWrite() const { return begin() + writer_; }

	size_t readableBytes() const { return writer_ - reader_; }
	size_t writableBytes() const { return buf_.size() - writer_; }
	size_t prependableBytes() const { return reader_; }

	void retrieve(size_t len){
		if (len < readableBytes())reader_ += len;
		else retrieveAll();
	}

	void retrieveAll() {
		reader_ = PREPEND;
		writer_ = PREPEND;
	}

	void moveReader(size_t len) {
		if (reader_ < writer_) {
			reader_ += len;
		}
		else {
			reader_ = writer_;
		}
	}

	void moveWriter(size_t len) {
		if (writer_ < buf_.size()) {
			writer_ += len;
		}
		else {
			makeSpace(len);
			writer_ += len;
		}
	}

	std::string retrieveAsString(size_t len) {
		if (len < readableBytes()) {
			std::string res(peek(), len);
			reader_ += len;
			return res;
		}
		else {
			return retrieveAllAsString();
		}
	}

	std::string retrieveAllAsString() {
		size_t len = readableBytes();
		std::string res(peek(), len);
		retrieveAll();
		return res;
	}

	bool isAdequate(size_t len) {
		return len < writableBytes();
	}

	void append(const std::string& str) {
		append(str.data(), str.size());
	}

	void append(const char* data, size_t len) {

		if (!isAdequate(len))makeSpace(len);
		std::copy(data, data + len, beginWrite());
		writer_ += len;
	}

	Buffer& operator+=(const std::string& str) {
		append(str);
		return *this;
	}

	Buffer& operator+=(const char* data) {
		append(data, strlen(data));
		return *this;
	}

	ssize_t readFd(int fd, int* saveErrno) {
		char extrabuf[65536] = { 0 };

		struct iovec iov[2];
		const size_t writable = writableBytes();

		iov[0].iov_base = begin() + writer_;
		iov[0].iov_len = writable;
		iov[1].iov_base = extrabuf;
		iov[1].iov_len = sizeof(extrabuf);

		const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
		const ssize_t n = ::readv(fd, iov, iovcnt);

		if (n < 0) {
			*saveErrno = errno;
		}
		else if (n <= writable) {
			writer_ += n;
		}
		else {
			writer_ = buf_.size();
			append(extrabuf, n - writable);
		}
		return n;
	}

	ssize_t writeFd(int fd, int* saveErrno) {
		ssize_t n = write(fd, peek(), readableBytes());
		if (n < 0) {
			*saveErrno = errno;
		}
		return n;
	}

private:

	void makeSpace(size_t len)
	{
		if (writableBytes() + prependableBytes() < len + PREPEND) {
			buf_.resize(writer_ + len);
		}
		else {
			size_t readable = readableBytes();
			std::copy(begin() + reader_, begin() + writer_, begin() + PREPEND);
			reader_ = PREPEND;
			writer_ = reader_ + readable;
		}
	}

	char* begin() {
		return &*buf_.begin();
	}

	const char* begin() const {
		return &*buf_.begin();
	}

	std::vector<char>buf_;
	size_t reader_;
	size_t writer_;
};

} //namespace carol

#endif //BASE_BUFFER_HPP