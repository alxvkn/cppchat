#include <iostream>
#include <thread>

#include "Socket.hpp"
#include "def.h"

#define PORT 1234

int main (int argc, char *argv[]) {
    Socket s;

    s.connect({"127.0.0.1", PORT});

    std::thread printer([&]{
        while (true) {
            auto msg = s.recv(256);
            if (msg.size() == 0)
                break;

            std::cout << std::string(msg.begin(), msg.end()) << std::endl;
        }
    });

    while (true) {
        std::string msg;
        std::cin >> msg;

        try {
            s.send(msg);
            dbgout("inside try");
        } catch (std::exception e) {
            dbgout("inside catch");
            std::cout << e.what();
            break;
        }
    }

    return 0;
}
