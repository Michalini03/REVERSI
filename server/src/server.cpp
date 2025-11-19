#include "../include/server.h"      // Include the header file we will create
#include "../include/lobby.h"       // Include the lobby header
#include <iostream>      // For standard I/O operations (cout, cerr)
#include <string>        // For using std::string
#include <cstring>       // For C-style string functions (memset)
#include <sys/socket.h>  // For socket functions (socket, bind, listen, accept)
#include <netinet/in.h>  // For sockaddr_in structure and INADDR_ANY
#include <unistd.h>      // For close() function
#include <sstream>       // For std::stringstream (you added this)
#include <vector>        // For std::vector (needed for parsing)
#include <algorithm>
#include <thread>
#include <mutex>

// Constants
#define PORT 9999
#define LOBBY_COUNT 5
#define PREFIX_GAME "REV"

// Global variables
std::vector<Lobby> lobbies; 
std::vector<int> client_sockets; 

std::mutex clients_mutex;
std::mutex lobbies_mutex;

void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0}; // Buffer to store received data
    
    for (int i = 0; i < LOBBY_COUNT; ++i) {
        lobbies.emplace_back(i);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // --- 2. Bind the socket to an address and port --
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // --- 3. Listen for incoming connections ---
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    while(true) {

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            // Don't exit, just log the error and continue listening
            continue; 
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            std::cout << "Connection accepted!" << std::endl;
            client_sockets.push_back(new_socket);
        }

        std::thread client_thread(handleClientLogic, new_socket);
        client_thread.detach(); 
    }

    close(server_fd);
}


void handleClientLogic(int client_socket) {
    char buffer[1024] = {0};

    while(true) {
        memset(buffer, 0, 1024);
        int valread = read(client_socket, buffer, 1024);
        
        if (valread <= 0) {
            std::cout << "Client " << client_socket << " disconnected." << std::endl;
            
            // TODO: Pause game if in a lobby
            {
                std::lock_guard<std::mutex> lock(lobbies_mutex);
                /*for (auto &lobby : lobbies) {
                    if (lobby.player1_socket == client_socket) lobby.player1_socket = -1;
                    if (lobby.player2_socket == client_socket) lobby.player2_socket = -1;
                }*/
            }
            break; 
        }

        handleMessage(client_socket, buffer);
    }

    close(client_socket);
    
    // Remove from global list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = std::find(client_sockets.begin(), client_sockets.end(), client_socket);
        if (it != client_sockets.end()) {
            client_sockets.erase(it);  // erase the single element
        }
    }
}

void handleMessage(int client_socket, const char* message) {
    // For now, just echo the message back to the client
    std::string messagePrefix = "REV";
    std::string messageStr(message);

    if (messageStr.substr(0, messagePrefix.size()).compare(messagePrefix) == 0) {
        // Strings are EQUAL (prefix IS "REV")
        std::cout << "Received a game-related message." << std::endl;
        std::stringstream ss(messageStr);

        std::string command;
        std::string prefix;
        ss >> prefix;
        ss >> command;

        if (command == "MOVE") {
            int x, y;
            ss >> x >> y;
            std::cout << "Processing MOVE command to (" << x << ", " << y << ")" << std::endl;
            // Here you would add logic to update the game state
        }
        else if (command == "CREATE") {
            std::cout << "Processing CREATE command." << std::endl;
            // Here you would add logic to create a new game
            if(sendLobbyList(client_socket) == 0) {
                std::cout << "Sent updated lobby list to client." << std::endl;
            } else {
                std::cout << "Failed to send lobby list to client." << std::endl;
            }
        }
        else if (command == "JOIN") {
            int result;
            int lobbyId;
            ss >> lobbyId;
            std::cout << "Processing JOIN command for lobby " << lobbyId << std::endl;

            result = handleLobbyJoin(client_socket, lobbyId);
            // Here you would add logic to join the specified lobby
        }
        else {
            std::cout << "Unknown game command: " << command << std::endl;
        }
    } else {
        // Strings are NOT equal (no prefix, must be lobby message)
        std::cout << "Received a lobby-related message." << std::endl;
    }

    std::cout << "Handling message: " << messageStr << std::endl;
    send(client_socket, message, strlen(message), 0);
}

int sendLobbyList(int client_socket) {
    std::string prefix(PREFIX_GAME);
    std::string lobbyList = prefix + " LOBBY " + std::to_string(LOBBY_COUNT) + "\n";
    
    send(client_socket, lobbyList.c_str(), lobbyList.size(), 0);
    return 0;
}

int handleLobbyJoin(int client_socket, int lobbyId) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
        return -1; // Invalid lobby ID
    }

    Lobby &lobby = lobbies[lobbyId];
    int result = lobby.appendPlayer(client_socket);
    return result; // 1 for player 1, 2 for player 2, 3 for full, -1 for error
}