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

#include "Socket.hpp"
#include "def.h"

// TODO: exceptions with explanations

Socket::Socket() {
    socket_fd = socket(PF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        throw std::exception();
    }
    dbgout("Socket() " << socket_fd);
}

// move constructor
Socket::Socket(Socket&& other) : socket_fd(other.socket_fd) {
    dbgout("Socket(Socket&& other) with other.socket_fd = " << socket_fd);
    other.socket_fd = -1;
}

Socket::Socket(int socket_fd) : socket_fd(socket_fd) {
    dbgout("Socket(int) " << socket_fd);
}

Socket::~Socket() {
    dbgout("~Socket() " << socket_fd);
    close();
}

void Socket::close() {
    dbgout("close() " << socket_fd);
    if (socket_fd == -1) return;
    ::close(socket_fd);
    socket_fd = -1;
}

void Socket::bind(Address address) {
    if (address.host.length() == 0)
        address.host = "0.0.0.0";

    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = inet_addr(address.host.c_str());
    socket_addr.sin_port = htons(address.port);

    if (::bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1) {
        throw std::exception();
    }
}

void Socket::connect(Address address) {
    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = inet_addr(address.host.c_str());
    socket_addr.sin_port = htons(address.port);

    if (::connect(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)))
        throw std::exception();
}

void Socket::listen(int queue) {
    if (::listen(socket_fd, queue)) {
        throw std::exception();
    }
}

std::pair<Socket, Socket::Address> Socket::accept() {
    struct sockaddr_in socket_addr;
    socklen_t addr_len = sizeof(socket_addr);

    int conn_fd = ::accept(socket_fd, (struct sockaddr*)&socket_addr, &addr_len);
    if (conn_fd == -1) {
        throw std::exception();
    }

    return {
        std::move(Socket(conn_fd)),
        Address { inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port) }
    };
}

std::vector<char> Socket::recv(size_t n, int flags) {
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

int Socket::send(const std::vector<char>& buf, int flags) {
    ssize_t n_sent = ::send(socket_fd, buf.data(), buf.size(), flags);

    if (n_sent == -1)
        throw std::exception();

    return n_sent;
}

int Socket::send(const std::string& str, int flags) {
    ssize_t n_sent = ::send(socket_fd, str.data(), str.length(), flags);

    dbgout("send(): n_sent = " << n_sent);
    if (n_sent == -1)
        throw std::exception();

    return n_sent;
}

Socket::Address Socket::getsockname() {
    struct sockaddr_in socket_addr;
    socklen_t addr_len;

    ::getsockname(socket_fd, (struct sockaddr*)&socket_addr, &addr_len);

    std::string address = inet_ntoa(socket_addr.sin_addr);
    unsigned short port = ntohs(socket_addr.sin_port);

    return { address, port };
}

Socket::Address Socket::getpeername() {
    struct sockaddr_in socket_addr;
    socklen_t addr_len;

    ::getpeername(socket_fd, (struct sockaddr*)&socket_addr, &addr_len);

    std::string address = inet_ntoa(socket_addr.sin_addr);
    unsigned short port = ntohs(socket_addr.sin_port);

    return { address, port };
}

void Socket::setsockopt(int level, int optname, int value) {
    ::setsockopt(socket_fd, level, optname, &value, sizeof(value));
}
