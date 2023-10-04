#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <set>

namespace coral {

class Socket;
class Send;
class Recv;
class Accept;

class Context {
public:
    Context();

    void run();
private:
    constexpr static std::size_t max_events = 10;
    const int fd_;
    friend Socket;
    friend Send;
    friend Recv;
    friend Accept;
    void Attach(Socket* socket);
    void WatchRead(Socket* socket);
    void UnwatchRead(Socket* socket);
    void WatchWrite(Socket* socket);
    void UnwatchWrite(Socket* socket);
    void Detach(Socket* socket);
};

} //namespace coral

#endif //IO_CONTEXT_H