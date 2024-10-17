#include <utility>
#include <string>
#include <vector>

class Socket {
public:
    struct Address {
        std::string host;
        unsigned short port;
    };

    Socket();

    // move constructor
    Socket(Socket&& other);

    Socket(int socket_fd);

    ~Socket();

    void close();

    void bind(Address);

    void listen(int queue);

    std::pair<Socket, Address> accept();

    std::vector<char> recv(size_t n, int flags = 0);

    int send(const std::vector<char>& buf, int flags = 0);

    Address getsockname();

    void setsockopt(int level, int optname, int value);

private:
    int socket_fd;
};
