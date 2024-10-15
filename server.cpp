#include <iostream>
#include <string>
#include <exception>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <utility>
#include <vector>

class Socket {
public:
    using Address = std::pair<std::string, unsigned short>;
    Socket() {
        socket_fd = socket(PF_INET, SOCK_STREAM, 0);

        if (socket_fd == -1) {
            throw std::exception();
        }
    }

    Socket(int socket_fd) : socket_fd(socket_fd) {}

    ~Socket() {
        close(socket_fd);
    }

    void bind(std::string host, unsigned short port) {
        struct sockaddr_in socket_addr;
        socket_addr.sin_family = AF_INET;
        socket_addr.sin_addr.s_addr = inet_addr(host.c_str());
        socket_addr.sin_port = htons(port);

        if (::bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1) {
            throw std::exception();
        }
    }

    void listen(int queue) {
        if (::listen(socket_fd, queue)) {
            throw std::exception();
        }
    }

    std::pair<Socket, Address> accept() {
        struct sockaddr_in socket_addr;
        socklen_t addr_len = sizeof(socket_addr);

        return {
            Socket(::accept(socket_fd, (struct sockaddr*)&socket_addr, &addr_len)),
            Address(inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port))
        };
    }

    std::vector<char> recv(int len, int flags = 0) {
        std::vector<char> buf;
        buf.reserve(len);
    }

    Address getsockname() {
        struct sockaddr_in socket_addr;
        socklen_t addr_len;

        ::getsockname(socket_fd, (struct sockaddr*)&socket_addr, &addr_len);

        std::string address = inet_ntoa(socket_addr.sin_addr);
        unsigned short port = ntohs(socket_addr.sin_port);

        return { address, port };
    }

    void setsockopt(int level, int optname, int value) {
        ::setsockopt(socket_fd, level, optname, &value, sizeof(value));
    }

private:
    int socket_fd;
};

int main (int argc, char *argv[]) {
    Socket s;
    s.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
    s.bind("0.0.0.0", 1234);
    s.listen(0);

    auto [ connection, address ] = s.accept();

    auto [ host, port ] = address;

    std::cout << host << ", " << port << std::endl;

    return 0;
}
