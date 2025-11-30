#include "../include/server.h"      // Include the header file we will create
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

#define PAUSE_STATUS 3
#define ENDED_STATUS 0

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

    // Can use port immediately after crash
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
    std::string dataBuffer = ""; 
    char tempBuffer[1024];

    // Create new player
    Player *new_player = new Player(client_socket);
    

    while(true) {
        memset(tempBuffer, 0, 1024);
        int valread = read(client_socket, tempBuffer, 1024);
        
        if (valread <= 0) {
            std::cout << "Client " << client_socket << " disconnected." << std::endl;
            
            // TODO: Pause game if in a lobby
            {
                std::lock_guard<std::mutex> lock(lobbies_mutex);
                for (auto &lobby : lobbies) {
                    if (lobby.getPlayerSocket1() == client_socket || lobby.getPlayerSocket2() == client_socket) 
                        lobby.removePlayer(client_socket);
                }
            }
            break; 
        }
        dataBuffer.append(tempBuffer, valread);
        size_t pos = 0;
        while ((pos = dataBuffer.find('\n')) != std::string::npos) {
            // Extract single message
            std::string message = dataBuffer.substr(0, pos);
            
            // Remove message from buffer
            dataBuffer.erase(0, pos + 1);

            // Process it
            handleMessage(client_socket, message.c_str(), *new_player);
        }

    }

    delete new_player;
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

void handleMessage(int client_socket, const char* message, Player& player) {
    // For now, just echo the message back to the client
    std::string messagePrefix = "REV";
    std::string messageStr(message);

    std::cout << "Handling message:" << messageStr << std::endl;

    if (messageStr.substr(0, messagePrefix.size()).compare(messagePrefix) == 0) {
        // Strings are EQUAL (prefix IS "REV")
        std::cout << "Received a game-related message." << std::endl;
        std::stringstream ss(messageStr);

        std::string command;
        std::string prefix;
        ss >> prefix;
        ss >> command;

        if (command == "CREATE") {
            std::string username;
            ss >> username;

            player.appendName(username);

            std::cout << "Processing CREATE command." << std::endl;

            // Look if user is not in game already
            {
                std::lock_guard<std::mutex> lock(lobbies_mutex);
                for (auto &lobby : lobbies) {
                    if (lobby.reconnectUser(player) != -1) {
                        std::cout << "[SERVER] User already connected to a lobby." << std::endl;
                        if (lobby.getStatus() == PAUSE_STATUS) {
                            // Resume game
                            std::cout << "[SERVER] Resuming paused game for user." << std::endl;
                            handleReconecting(client_socket, player, lobby);
                            return;
                        }
                    }
                }
            }

            // Here you would add logic to create a new game
            if(sendLobbyList(client_socket) == 0) {
                std::cout << "[SERVER] Sent updated lobby list to client." << std::endl;
                // return username;
            } else {
                std::cout << "[ERROR] Failed to send lobby list to client." << std::endl;
            }
        }
        else if (command == "JOIN") {
            int result;
            int lobbyId;
            ss >> lobbyId;
            std::cout << "Processing JOIN command for lobby " << lobbyId << std::endl;

            lobbyId--;
            result = handleLobbyJoin(client_socket, lobbyId, player);
            // Here you would add logic to join the specified lobby
            if(result == 1) {
                std::cout << "[SERVER] Client joined as Player 1 in lobby " << lobbyId << std::endl;
            } else if(result == 2) {
                std::cout << "[SERVER] Client joined as Player 2 in lobby " << lobbyId << std::endl;
                startGame(lobbyId);
            } else if(result == 3) {
                std::cout << "[SERVER] Lobby " << lobbyId << " is full." << std::endl;
            } else {
                std::cout << "[SERVER] Error joining lobby " << lobbyId << std::endl;
            }
        }
        else if (command == "EXIT") {
            int lobbyId;
            ss >> lobbyId;
            std::cout << "Processing EXIT command for lobby " << lobbyId << std::endl;

            lobbyId--;
            handleLobbyExit(client_socket, lobbyId);
            // Here you would add logic to exit the specified lobby
        }
        else if (command == "MOVE") {
            int x = -1, y = -1, lobbyId = -1;

            if (ss >> x >> y >> lobbyId) {
                std::cout << "Processing MOVE command to (" << x << ", " << y << ")" << std::endl;
                
                lobbyId--; 

                if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
                    std::cerr << "[ERROR] Invalid Lobby ID: " << lobbyId << std::endl;
                    return;
                }

                std::cout << "HII (" << x << ", " << y << ") in Lobby " << lobbyId << std::endl;         
                handleMoving(x, y, client_socket, lobbyId);
            } else {
                std::cerr << "[ERROR] Failed to parse MOVE command args." << std::endl;
            }
        }
        else {
            std::cout << "Unknown game command: " << command << std::endl;
        }
    } else {
        return;
    }
}

int sendConnectInfo(int client_socket, int playerNumber) {
    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " CONNECT " + std::to_string(playerNumber) + "\n";
    
    send(client_socket, message.c_str(), message.size(), 0);
    return 0;
}

int sendStartingPlayerInfo(int client_socket, std::string player1, std::string player2, int playerNumber, Lobby& lobby) {
    std::string prefix(PREFIX_GAME);

    // Send START message
    std::string message = prefix + " START " + std::to_string(playerNumber);
    message += " " + player1 + " " + player2;
    message += "\n";
    send(client_socket, message.c_str(), message.size(), 0);

    sendState(client_socket, lobby);
    return 0;
}

int sendState(int client_socket, Lobby& lobby) {
    std::string prefix(PREFIX_GAME);

    if (client_socket < 0 || !&lobby) {
        return -1;
    }

    std::string boardState = prefix + " STATE " + lobby.getBoardStateString() + "\n";
    send(client_socket, boardState.c_str(), boardState.size(), 0);
    return 0;
}

int sendLobbyList(int client_socket) {
    std::string prefix(PREFIX_GAME);
    std::string lobbyList = prefix + " LOBBY " + std::to_string(LOBBY_COUNT) + "\n";
    
    send(client_socket, lobbyList.c_str(), lobbyList.size(), 0);
    return 0;
}

int handleLobbyJoin(int client_socket, int lobbyId, Player& player) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
        return -1; // Invalid lobby ID
    }

    std::lock_guard<std::mutex> lock(lobbies_mutex);
    for (auto &lobby : lobbies) {
        if (lobby.isUserConnected(client_socket)) {
            std::cout << "[SERVER] User already connected to a lobby." << std::endl;
            return -1;
        }
    }
    
    Lobby &lobby = lobbies[lobbyId];
    int result = lobby.setPlayer(&player);

    if(result > 0) {
        sendConnectInfo(client_socket, result);
    }
    
    return result;
}

int handleLobbyExit(int client_socket, int lobbyId) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
        return -1; // Invalid lobby ID
    }

    if (client_socket < 0) {
        return -1; // Invalid socket
    }

    std::lock_guard<std::mutex> lock(lobbies_mutex);
    std::cout << "AHOJ" << std::endl;
    Lobby &lobby = lobbies[lobbyId];
    lobby.removePlayer(client_socket);
    return 0;
}

int handleMoving(int x, int y, int client_socket, int lobbyId) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
        return -1; // Invalid lobby ID
    }

    if (x < 0 || x >=8 || y < 0 || y >=8) {
        return -1; // Invalid move coordinates
    }

    if (client_socket < 0) {
        return -1; // Invalid socket
    }
    
    std::lock_guard<std::mutex> lock(lobbies_mutex);
    Lobby &lobby = lobbies[lobbyId];

    int current_player = lobby.canUserPlay(client_socket);
    if(current_player == -1) {
        std::cout << "[LOBBY " << lobbyId << "] It's not the player's turn or player not found." << std::endl;
        return -1;
    }

    if (lobby.validateAndApplyMove(x, y, current_player)) {
        std::cout << "JSEM TADY" << std::ends;

        lobby.setStatus((current_player == 1) ? 2 : 1);

        int client_socket_1 = lobby.getPlayerSocket1();
        int client_socket_2 = lobby.getPlayerSocket2();

        sendState(client_socket_1, lobby);
        sendState(client_socket_2, lobby);
        return 0;
    }
    else {
        return 0;
    }
}

int handleReconecting(int client_socket, Player& player, Lobby& lobby) {
    if(client_socket < 0 || &player == nullptr || &lobby == nullptr) {
        return -1;
    }

    lobby.reconnectUser(player);
    sendStartingPlayerInfo(client_socket, lobby.getPlayer1Username(), lobby.getPlayer2Username(), lobby.getStatus(), lobby);
    return 0;
}



void startGame(int lobbyIndex) {
    std::lock_guard<std::mutex> lock(lobbies_mutex);
    if(lobbyIndex < 0 || lobbyIndex >= LOBBY_COUNT) return;

    Lobby &lobby = lobbies[lobbyIndex];
    lobby.setStatus(1); 

    // Use Safe Getters to prevent SegFault
    std::string p1Name = lobby.getPlayer1Username();
    std::string p2Name = lobby.getPlayer2Username();
    int p1Socket = lobby.getPlayerSocket1();
    int p2Socket = lobby.getPlayerSocket2();

    std::cout << "[LOBBY " << (lobbyIndex+1) << "] Game started between " 
              << p1Name << " and " << p2Name << std::endl;
    std::cout << "[LOBBY " << (lobbyIndex+1) << "] Player 1 socket: " << p1Socket << ", Player 2 socket: " << p2Socket << std::endl;

    // Send info to Player 1 (You are P1, Opponent is P2)
    sendStartingPlayerInfo(p1Socket, p1Name, p2Name, 1, lobby); 
    sendStartingPlayerInfo(p2Socket, p1Name, p2Name, 1, lobby);
}