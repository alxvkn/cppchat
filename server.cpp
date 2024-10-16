#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <string>
#include <exception>
#include <utility>
#include <vector>

#define DEBUG

#ifdef DEBUG
#define dbgout(expr) do { std::cout << expr << std::endl; } while (0)
#else
#define dbgout(expr)
#endif

class Socket {
public:
    using Address = std::pair<std::string, unsigned short>;
    Socket() {
        socket_fd = socket(PF_INET, SOCK_STREAM, 0);

        if (socket_fd == -1) {
            throw std::exception();
        }
    }

    // move constructor
    Socket(Socket&& other) : socket_fd(other.socket_fd) {
        dbgout("Socket(Socket&& other) with other.socket_fd = " << socket_fd);
        other.socket_fd = -1;
    }

    Socket(int socket_fd) : socket_fd(socket_fd) {
        dbgout("Socket(int) " << socket_fd);
    }

    ~Socket() {
        dbgout("~Socket() " << socket_fd);
        close();
    }

    void close() {
        dbgout("close() " << socket_fd);
        if (socket_fd == -1) return;
        ::close(socket_fd);
        socket_fd = -1;
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

        int conn_fd = ::accept(socket_fd, (struct sockaddr*)&socket_addr, &addr_len);
        if (conn_fd == -1) {
            throw std::exception();
        }

        return {
            std::move(Socket(conn_fd)),
            Address(inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port))
        };
    }

    std::vector<char> recv(size_t n, int flags = 0) {
        std::vector<char> buf;
        buf.reserve(n);

        size_t n_read = ::recv(socket_fd, buf.data(), n, flags);
        dbgout("recv(): n_read = " << n_read);

        if (n_read == -1) {
            std::cout << strerror(errno) << std::endl;
            std::cout << socket_fd << std::endl;
            throw std::exception();
        }

        for (unsigned i = 0; i < n_read; i++) {
            buf.push_back(buf.data()[i]);
        }

        return buf;
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

    std::vector<char> msg = connection.recv(32);

    for (char c : msg) {
        std::cout << c;
    }
    std::cout << std::endl;

    return 0;
}
