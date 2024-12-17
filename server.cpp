#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <memory>
#include <format>

#include <sys/socket.h>
#include <unordered_map>

#include "Socket.hpp"
#include "def.h"

#include "protocol.hpp"

class Server {

    std::unordered_map<std::string, std::shared_ptr<Socket>> users;

    void broadcast_message(const std::string& username, const std::string& msg) {
        for (auto user : users) {
            if (user.first != username)
                user.second->send(std::format("{}: {}\n", username, msg));
        }
    }

    void client_worker(std::shared_ptr<Socket> connection_ptr) {

        std::string nickname;
        std::vector<char> msg;

        connection_ptr->send("please, enter your nickname: ");

        while (true) {
            msg = connection_ptr->recv(256);
            nickname = std::string(msg.begin(), msg.end());

            if (!users.count(nickname))
                break;

            connection_ptr->send("this nickname is currently taken,\n"
                                 "please choose another one: ");
        }

        users[nickname] = connection_ptr;
        connection_ptr->send(std::format(
            "you were successfully connected with the nickname {}\n",
            nickname
        ));

        const std::string prompt = "> ";

        connection_ptr->send(prompt);
        while ((msg = connection_ptr->recv(256)).size() != 0) {
            std::string msg_string = std::string(msg.begin(), msg.end());

            std::cout << nickname << ": " << msg_string << std::endl;
            if (msg_string == "/q")
                break;

            broadcast_message(nickname, msg_string);

            connection_ptr->send(std::format("\n{}", prompt));
        }

        users.erase(nickname);
    }

public:
    const Socket::Address address_;

    Server(const Socket::Address& address = { "0.0.0.0", 0 }) : address_(address) {}

    void run() {
        Socket s;
        s.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
        s.bind(address_);
        s.listen(0);

        Socket::Address bound_address = s.getsockname();
        dbgout(std::format(
            "server is listening on {}:{}",
            bound_address.host,
            bound_address.port
        ));

        std::vector<std::thread> workers;

        std::string command;

        std::atomic<bool> end = false;

        std::thread accepter([&]{
            while (not end) {
                auto [ c, address ] = s.accept();

                auto [ host, port ] = address;

                dbgout(host << ", " << port);

                workers.push_back(std::thread(
                    &Server::client_worker,
                    this,
                    std::make_shared<Socket>(std::move(c))
                ));
            }
        });

        while (true) {
            std::cout << "> ";
            std::cin >> command;
            if (command == "q")
                break;
        }

        end = true;

    }

};


int main (int argc, char *argv[]) {
    Server server = Server({"", 1234});

    server.run();

    return 0;
}
