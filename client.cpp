#include <iostream>
#include <string>
#include <thread>

#include "Socket.hpp"
#include "def.h"

#define PORT 1234

int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "provide server host and port as positional arguments" << std::endl;
        return 1;
    }
    Socket s;

    s.connect({argv[1], (unsigned short)std::stoul((argv[2]))});

    std::thread printer([&]{
        while (true) {
            std::vector<char> msg = s.recv(256);
            if (msg.size() == 0)
                break;

            std::cout << std::string(msg.begin(), msg.end()) << std::flush;
        }
    });

    while (true) {
        std::string msg;
        std::getline(std::cin, msg);

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
