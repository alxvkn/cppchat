#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <memory>

#include <sys/socket.h>
#include <unordered_map>

#include "Socket.hpp"
#include "def.h"

class User {
public:
    User(std::string username, std::string password) : username(username) {
        password_hash = std::hash<std::string>{}(password);
    }

    User(std::string username, size_t password_hash)
        : username(username), password_hash(password_hash) {}

    std::string username;
    size_t password_hash;
};

class UserManager {
    std::unordered_map<std::string, std::shared_ptr<Socket>> connected_users;
    std::unordered_map<std::string, size_t> password_hashes;
    std::string filename;
public:
    UserManager() = default;
    UserManager(std::string filename) : filename(filename) {
        std::ifstream in(filename);
        std::string line;

        while (std::getline(in, line)) {
            std::stringstream line_stream(line);
            std::string username;
            size_t password_hash;

            line_stream >> username >> password_hash;

            password_hashes[username] = password_hash;
        }
    }
    ~UserManager() {
        save();
    }

    void save() {
        std::ofstream out(filename);

        for (auto [ username, password_hash ] : password_hashes) {
            out << username << " " << password_hash << "\n";
        }

        out.close();
    }

    bool user_exists(std::string username) {
        return password_hashes[username];
    }

    bool login(std::shared_ptr<Socket> connection, std::string username, std::string password) {
        User user = User(username, password);

        if (password_hashes[user.username] == user.password_hash) {
            connected_users[user.username] = std::move(connection);

            return true;
        }
        return false;
    }

    void logout(std::string username) {
        connected_users.erase(username);
    }

    void sign_up(std::string username, std::string password) {
        User user = User(username, password);
        password_hashes[user.username] = user.password_hash;
        std::cout << "sign_up: " << password_hashes.size()
            << username << std::endl;
        std::cout << "sign_up: " << connected_users.size() << std::endl;
    }
};

UserManager user_manager("users.txt");

std::string help(
    "available commands:\n"
    "/q - disconnect\n"
    "/name <username> - change username"
);

void client_worker(std::shared_ptr<Socket> connection_ptr) {

    std::vector<char> msg;

    connection_ptr->send("please, enter your username");
    msg = connection_ptr->recv(256);
    std::string username, password;
    username = std::string(msg.begin(), msg.end());

    if (user_manager.user_exists(username)) {
        connection_ptr->send("now your password");
        msg = connection_ptr->recv(256);
        password = std::string(msg.begin(), msg.end());
    } else {
        connection_ptr->send("you are not registered, please come up with a password");
        msg = connection_ptr->recv(256);
        password = std::string(msg.begin(), msg.end());
        user_manager.sign_up(username, password);
    }
    if (!user_manager.login(connection_ptr, username, password))
        return;

    while ((msg = connection_ptr->recv(256)).size() != 0) {
        std::string msg_string = std::string(msg.begin(), msg.end());

        std::cout << msg_string << std::endl;
        if (msg_string == "/q")
            break;
        else if (msg_string == "/help")
            connection_ptr->send(help);
        else {
        }
    }

    user_manager.logout(username);
}

int main (int argc, char *argv[]) {
    Socket s;
    s.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);
    s.bind({ "0.0.0.0", 1234 });
    s.listen(0);

    std::vector<std::thread> workers;

    std::string command;

    std::atomic<bool> end = false;

    std::thread accepter([&]{
        while (not end) {
            auto [ c, address ] = s.accept();

            auto [ host, port ] = address;

            dbgout(host << ", " << port);

            workers.push_back(std::thread(
                client_worker,
                std::make_shared<Socket>(std::move(c))
            ));
        }
    });

    while (true) {
        std::cout << "> ";
        std::cin >> command;
        if (command == "q")
            break;
        else if (command == "save")
            user_manager.save();
    }

    end = true;

    return 0;
}
