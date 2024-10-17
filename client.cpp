#include <iostream>

#include "Socket.hpp"
#include "def.h"

#define PORT 1234

int main (int argc, char *argv[]) {
    Socket s;

    s.connect({"127.0.0.1", PORT});

    while (true) {
        std::string msg;
        std::cout << "> ";
        std::cin >> msg;

        try {
            s.send(msg);
            dbgout("inside try");
        } catch (std::exception e) {
            dbgout("inside catch");
            std::cout << e.what();
            break;
        }

        auto response = s.recv(32);

        if (response.size() == 0)
            break;

        std::cout << std::string(response.begin(), response.end()) << std::endl;
    }

    return 0;
}
