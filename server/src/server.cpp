#include "../include/server.h"      // Include the header file we will create
#include <iostream>      // For standard I/O operations (cout, cerr)
#include <string>        // For using std::string
#include <cstring>       // For C-style string functions (memset)
#include <sys/socket.h>  // For socket functions (socket, bind, listen, accept)
#include <netinet/in.h>  // For sockaddr_in structure and INADDR_ANY
#include <unistd.h>      // For close() function
#include <sstream>       // For std::stringstream (you added this)
#include <vector>        // For std::vector (needed for parsing)

// Constants
#define PORT 9999
#define LOBBY_COUNT 5
#define PREFIX_GAME "REV"

void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0}; // Buffer to store received data

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // --- (Optional) Set socket options ---
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // --- 2. Bind the socket to an address and port ---
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

    // --- 4. Start the infinite loop to accept clients ---
    while(true) {
        std::cout << "\nWaiting for a new connection..." << std::endl;

        // --- 4a. Accept a new connection ---
        // This is a blocking call. The program will wait here until a client connects.
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            // Don't exit, just log the error and continue listening
            continue; 
        }

        std::cout << "Connection accepted!" << std::endl;


        while(true) {
            // Clear the buffer
            memset(buffer, 0, 1024);
            int valread = read(new_socket, buffer, 1024);
            
            if (valread <= 0) {
                // If read returns 0 or -1, the client has disconnected or an error occurred
                std::cout << "Client disconnected." << std::endl;
                break; // Exit the inner loop
            } else {
                std::cout << "Client said: " << buffer << std::endl;
            }

            handleMessage(new_socket, buffer);
        }

        // --- 4d. Close the client-specific socket ---
        // We're done with this client, close their socket and loop back to accept()
        close(new_socket);
    }
    close(server_fd);
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