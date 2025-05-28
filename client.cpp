#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        return 1;
    }
    const char* ip = argv[1];
    int port = std::atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number." << std::endl;
        return 1;
    }

    std::cout << "Connecting to " << ip << ":" << port << "..." << std::endl;
    std::cout << std::flush;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0){
        std::cout << " Connected!" << std::endl;
    }else{
        std::cout << " Connection failed!" << std::endl;
    }

    char buf[BUFFER_SIZE];
    ssize_t n = recv(sock, buf, sizeof(buf)-1, 0);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << buf;
    }

    while (true) {
        std::cout << "\nYour guess (1-100): " << std::flush;
        std::string guess;
        if (!std::getline(std::cin, guess) || guess.empty()) continue;
        send(sock, guess.c_str(), guess.size(), 0);

        ssize_t len = recv(sock, buf, sizeof(buf)-1, 0);
        if (len <= 0) break;
        buf[len] = '\0';
        std::string resp(buf);
        if (resp.find("Too low") != std::string::npos || resp.find("Too high") != std::string::npos)
            std::cout << resp;
        else if (resp.find("Correct") != std::string::npos) {
            std::cout << resp << " You win!" << std::endl;
            break;
        } else
            std::cout << resp;
    }
    close(sock);
    return 0;
}