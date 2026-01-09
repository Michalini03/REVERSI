#include "./include/server.h" // Include the server header
#include <iostream>
#include <string>
#include <stdexcept>
#include <signal.h>

int main(int argc, char* argv[]) {

    signal(SIGPIPE, SIG_IGN);
    
    try {
        std::string serverIP = "";
        int serverPort = 0;

        if (argc == 3) {
            serverIP = argv[1];
            try {
                serverPort = std::stoi(argv[2]);
            } catch (const std::exception& e) {
                throw std::runtime_error("Invalid port number provided.");
            }
        } else if (argc != 1) {
            std::cout << "[WARNING] Usage: " << argv[0] << " <IP> <PORT>" << std::endl;
            return 1;
        }

        startServer(serverIP, serverPort);

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] A critical error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}