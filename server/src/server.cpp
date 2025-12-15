#include "../include/server.h"
#include "../include/handler.h"
#include "../include/sender.h"
#include "../include/global.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <algorithm>

// Define Globals Here
std::vector<Lobby> lobbies; 
std::vector<int> clientSockets; 
std::mutex clients_mutex;
std::mutex lobbies_mutex;

void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    for (int i = 0; i < LOBBY_COUNT; ++i) {
        lobbies.emplace_back(i);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    while(true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue; 
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            std::cout << "Connection accepted!" << std::endl;
            clientSockets.push_back(new_socket);
        }

        std::thread client_thread(handleClientLogic, new_socket);
        client_thread.detach(); 
    }

    close(server_fd);
}

void handleClientLogic(int clientSocket) {
    std::string dataBuffer = ""; 
    char tempBuffer[1024];

    // Create new player
    Player *new_player = nullptr;

    while(true) {
        memset(tempBuffer, 0, 1024);
        int valread = read(clientSocket, tempBuffer, 1024);
        
        if (valread <= 0 || (new_player != nullptr && new_player->tolerance > 3)) {
            std::cout << "Client " << clientSocket << " disconnected." << std::endl;
            
            {
                std::lock_guard<std::mutex> lock(lobbies_mutex);
                bool memoryRetained = false;
                for (auto &lobby : lobbies) {
                    if (lobby.getPlayerSocket1() == clientSocket || lobby.getPlayerSocket2() == clientSocket) {
                        lobby.removePlayer(clientSocket);
                        if(lobby.getStatus() == PAUSE_STATUS) {
                            memoryRetained = true;
                            int disconnected_user = -1;
                            int connected_oponent_socket = - 1;
                            if(lobby.getPlayerSocket1() == -1) {
                                disconnected_user = 1;
                                connected_oponent_socket = lobby.getPlayerSocket2();
                            }
                            else if (lobby.getPlayerSocket2() == -1) {
                                disconnected_user = 2;
                                connected_oponent_socket = lobby.getPlayerSocket1();
                            }
                            sendDisconnectInfo(connected_oponent_socket, disconnected_user);
                            break;
                        }
                    }
                }
                if (!memoryRetained) delete new_player;
            }
            break; 
        }

        if (new_player == nullptr) {
            new_player = new Player(clientSocket);
        }

        dataBuffer.append(tempBuffer, valread);
        size_t pos = 0;
        while ((pos = dataBuffer.find('\n')) != std::string::npos) {
            std::string message = dataBuffer.substr(0, pos);
            dataBuffer.erase(0, pos + 1);
            handleMessage(clientSocket, message.c_str(), *new_player);
        }
    }

    close(clientSocket);
    
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
        if (it != clientSockets.end()) {
            clientSockets.erase(it);
        }
    }
}