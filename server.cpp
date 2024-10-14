#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class Socket {
    int socket_fd;

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

    void bind(std::string addr, unsigned short port) {
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(addr.c_str());
        server_addr.sin_port = htons(port);

        if (::bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            throw std::exception();
        }
    }

    void listen(int queue) {
        if (::listen(socket_fd, queue)) {
            throw std::exception();
        }
    }

    Socket accept() {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        return Socket(::accept(socket_fd, (struct sockaddr*)&client_addr, &client_addr_len));
    }
};

int main (int argc, char *argv[]) {
    return 0;
}
