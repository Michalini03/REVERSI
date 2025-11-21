#include "../include/server.h"      // Include the header file we will create
#include "../include/lobby.h"       // Include the lobby header
#include "../include/player.h"      // Include the player header
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
std::vector<Player> players;

std::mutex clients_mutex;
std::mutex lobbies_mutex;
std::mutex players_mutex;

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
    int player_id;

    // Create new player
    {
        std::lock_guard<std::mutex> lock(players_mutex);
        Player new_player(client_socket, players.size());
        player_id = new_player.id;
        players.push_back(new_player);
        std::cout << "[SERVER] New player created with socket " << client_socket << std::endl;
    }

    while(true) {
        memset(buffer, 0, 1024);
        int valread = read(client_socket, buffer, 1024);
        
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

        handleMessage(client_socket, player_id, buffer);
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

void handleMessage(int client_socket, int player_id, const char* message) {
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
            std::string username;
            ss >> username;

            std::lock_guard<std::mutex> lock(players_mutex);
            players[player_id].appendName(username);

            std::cout << "Processing CREATE command." << std::endl;

            // Look if user is not in game already
            {
                std::lock_guard<std::mutex> lock(lobbies_mutex);
                for (auto &lobby : lobbies) {
                    if (lobby.reconnectUser(players[player_id]) != -1) {
                        std::cout << "[SERVER] User already connected to a lobby." << std::endl;
                        if (lobby.getStatus() == PAUSE_STATUS) {
                            // Resume game
                            std::cout << "[SERVER] Resuming paused game for user." << std::endl;
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

            result = handleLobbyJoin(client_socket, player_id, lobbyId);
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
        else if (command == "MOVE") {
            int x, y;
            ss >> x >> y;
            std::cout << "Processing MOVE command to (" << x << ", " << y << ")" << std::endl;
            // Here you would add logic to update the game state
        }
        else {
            std::cout << "Unknown game command: " << command << std::endl;
        }
    } else {
        return;
    }

    std::cout << "Handling message: " << messageStr << std::endl;
    send(client_socket, message, strlen(message), 0);
}

int sendConnectInfo(int client_socket, int playerNumber) {
    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " CONNECT " + std::to_string(playerNumber) + "\n";
    
    send(client_socket, message.c_str(), message.size(), 0);
    return 0;
}

int sendStartingPlayerInfo(int client_socket, std::string player1, std::string player2, int playerNumber) {
    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " START " + std::to_string(playerNumber);
    message += " " + player1 + " " + player2;
    message += "\n";
    
    send(client_socket, message.c_str(), message.size(), 0);
    return 0;
}

int sendLobbyList(int client_socket) {
    std::string prefix(PREFIX_GAME);
    std::string lobbyList = prefix + " LOBBY " + std::to_string(LOBBY_COUNT) + "\n";
    
    send(client_socket, lobbyList.c_str(), lobbyList.size(), 0);
    return 0;
}

int handleLobbyJoin(int client_socket, int player_id, int lobbyId) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
        return -1; // Invalid lobby ID
    }

    std::lock_guard<std::mutex> lock(lobbies_mutex);
    std::lock_guard<std::mutex> lock2(players_mutex);
    for (auto &lobby : lobbies) {
        if (lobby.isUserConnected(client_socket)) {
            std::cout << "[SERVER] User already connected to a lobby." << std::endl;
            return -1;
        }
    }
    
    Lobby &lobby = lobbies[lobbyId];
    Player* player = &players[player_id];
    int result = lobby.appendPlayer(player);

    if(result > 0) {
        sendConnectInfo(client_socket, result);
    }
    
    return result;
}

int handleReconecting() {
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
    sendStartingPlayerInfo(p1Socket, p1Name, p2Name, 1); 
    
    // Send info to Player 2 (You are P2, Opponent is P1)
    sendStartingPlayerInfo(p2Socket, p2Name, p1Name, 1); 
}