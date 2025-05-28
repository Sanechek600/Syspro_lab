#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>

#define MIN_NUMBER 1
#define MAX_NUMBER 100
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number." << std::endl;
        return 1;
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    signal(SIGPIPE, SIG_IGN);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        char cip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, cip, sizeof(cip));
        int cport = ntohs(client_addr.sin_port);

        std::cout << "Client connected: " << cip << ":" << cport << std::endl;

        int secret = MIN_NUMBER + std::rand() % (MAX_NUMBER - MIN_NUMBER + 1);
        bool guessed = false;
        char buffer[BUFFER_SIZE];

        send(client_fd, std::string("Welcome to Guess-the-Number!\n").c_str(), 27, 0);

        while (!guessed) {
            ssize_t r = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
            if (r <= 0) {
                std::cout << "Halt " << cip << ":" << cport << " disconnected." << std::endl;
                break;
            }
            buffer[r] = '\0';
            long val = std::strtol(buffer, nullptr, 10);
            std::string resp;
            if (val < secret) {
                std::cout << "Client guessed " << val << " against " << secret << ". Too low" << std::endl;
                resp = "Too low! Try higher.\n";
            } else if (val > secret) {
                std::cout << "Client guessed " << val << " against " << secret << ". Too high" << std::endl;
                resp = "⇩ Too high! Try lower.\n";
            } else {
                std::cout << "Client guessed " << val << " against " << secret << ". Correct" << std::endl;
                resp = "Correct!\n";
                guessed = true;
            }
            send(client_fd, resp.c_str(), resp.size(), MSG_NOSIGNAL);
        }
        close(client_fd);
    }
    close(server_fd);
    return 0;
}