#include <iostream>

#include <sys/socket.h>

#include "Socket.hpp"

#define DEBUG

#ifdef DEBUG
#define dbgout(expr) do { std::cout << expr << std::endl; } while (0)
#else
#define dbgout(expr)
#endif

int main (int argc, char *argv[]) {
    Socket s;
    s.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
    s.bind("0.0.0.0", 1234);
    s.listen(0);

    auto [ c, address ] = s.accept();

    auto [ host, port ] = address;

    dbgout(host << ", " << port);

    std::vector<char> msg = c.recv(32);

    std::cout << std::string(msg.begin(), msg.end()) << std::endl;

    std::cout << "response > ";
    std::string response;
    std::cin >> response;
    c.send(std::vector<char>(response.begin(), response.end()));

    return 0;
}
