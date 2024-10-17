#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>

#include <sys/socket.h>

#include "Socket.hpp"
#include "def.h"

class User {
public:
    User(std::string username, std::string password) : username(username) {
        password_hash = std::hash<std::string>{}(password);
    }

    std::string username;
    size_t password_hash;
};

class UserManager {
public:
    bool login(std::string username, std::string password);
};

void client_handler(Socket connection) {
    std::vector<char> msg;
    while ((msg = connection.recv(32)).size() != 0) {
        std::string msg_string = std::string(msg.begin(), msg.end());

        std::cout << msg_string << std::endl;
        if (msg_string == "/q")
            break;

        connection.send(std::string(msg_string.rbegin(), msg_string.rend()));
    }
}

int main (int argc, char *argv[]) {
    Socket s;
    s.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
    s.bind({ "0.0.0.0", 1234 });
    s.listen(0);

    std::vector<std::thread> threads;

    while (true) {
        auto [ c, address ] = s.accept();

        auto [ host, port ] = address;

        dbgout(host << ", " << port);

        threads.push_back(std::thread(client_handler, std::move(c)));
    }

    return 0;
}
