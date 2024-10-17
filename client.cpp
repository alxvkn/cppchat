#include <iostream>

#include "Socket.hpp"

#define PORT 1234

int main (int argc, char *argv[]) {
    Socket s;

    s.connect({"127.0.0.1", PORT});

    std::string msg;
    std::cout << "msg > ";
    std::cin >> msg;

    s.send(msg);

    auto response = s.recv(32);
    std::cout << std::string(response.begin(), response.end()) << std::endl;

    return 0;
}
