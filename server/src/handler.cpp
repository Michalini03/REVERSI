#include "../include/handler.h"
#include "../include/sender.h" // Handler needs to send responses
#include "../include/global.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>

// Note: Globals (lobbies, mutexes) are extern'd in globals.h

void handleMessage(int client_socket, const char* message, Player& player) {
    std::string messagePrefix = "REV";
    std::string messageStr(message);

    if (messageStr.substr(0, messagePrefix.size()).compare(messagePrefix) == 0) {
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

            // Check reconnection
            {
                std::lock_guard<std::mutex> lock(lobbies_mutex);
                for (auto &lobby : lobbies) {
                    int connectedUser = lobby.reconnectUser(player);
                    if (connectedUser != -1) {
                        std::cout << "[SERVER] User already connected to a lobby." << std::endl;
                        std::cout << "[SERVER] Resuming paused game for user." << std::endl;
                        handleReconecting(client_socket, player, lobby, connectedUser);
                        return;
                    }
                }
            }

            if(sendLobbyList(client_socket) == 0) {
                std::cout << "[SERVER] Sent updated lobby list to client." << std::endl;
            } else {
                std::cout << "[ERROR] Failed to send lobby list to client." << std::endl;
            }
        }
        else if (command == "JOIN") {
            int result;
            int lobbyId;
            ss >> lobbyId;
            std::cout << "Processing JOIN command for lobby " << lobbyId << std::endl;

            result = handleLobbyJoin(client_socket, lobbyId, player);
            
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

            handleLobbyExit(client_socket, lobbyId);
        }
        else if (command == "MOVE") {
            int x = -1, y = -1, lobbyId = -1;

            if (ss >> x >> y >> lobbyId) {
                std::cout << "Processing MOVE command to (" << x << ", " << y << ")" << std::endl;

                if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
                    std::cerr << "[ERROR] Invalid Lobby ID: " << lobbyId << std::endl;
                    return;
                }

                handleMoving(x, y, client_socket, lobbyId);
            } else {
                std::cerr << "[ERROR] Failed to parse MOVE command args." << std::endl;
            }
        }
        else {
            std::cout << "Unknown game command: " << command << std::endl;
        }
    }
}

int handleLobbyJoin(int client_socket, int lobbyId, Player& player) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
        return -1; 
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
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) return -1;
    if (client_socket < 0) return -1;

    std::lock_guard<std::mutex> lock(lobbies_mutex);
    Lobby &lobby = lobbies[lobbyId];
    lobby.removePlayer(client_socket);
    return 0;
}

int handleMoving(int x, int y, int client_socket, int lobbyId) {
      if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) return -1;
      if (x < 0 || x >=8 || y < 0 || y >=8) return -1; 
      if (client_socket < 0) return -1; 
      
      std::lock_guard<std::mutex> lock(lobbies_mutex);
      Lobby &lobby = lobbies[lobbyId];

      int current_player = lobby.canUserPlay(client_socket);
      if(current_player == -1) {
            std::cout << "[LOBBY " << lobbyId << "] It's not the player's turn or player not found." << std::endl;
            return -1;
      }

      if (lobby.validateAndApplyMove(x, y, current_player)) {
            int client_socket_1 = lobby.getPlayerSocket1();
            int client_socket_2 = lobby.getPlayerSocket2();

            sendState(client_socket_1, lobby);
            sendState(client_socket_2, lobby);

            int newStatus = lobby.getStatus();

            if (newStatus == ENDED_STATUS) {
                  std::string msg = "REV END " + std::to_string(lobby.calculateWinner()) + "\n";
                  send(client_socket_1, msg.c_str(), msg.size(), 0);
                  send(client_socket_2, msg.c_str(), msg.size(), 0);
            }
            else if (newStatus == current_player) {
                  std::string msg = "REV PASS\n"; 
                  send(client_socket_1, msg.c_str(), msg.size(), 0);
                  send(client_socket_2, msg.c_str(), msg.size(), 0);
            }
            
            return 0;
      }
      else {
            return 0;
      }
}

int handleReconecting(int client_socket, Player& player, Lobby& lobby, int connectedUser) {
      if(client_socket < 0 || &player == nullptr || &lobby == nullptr) {
            return -1;
      }

      sendStartingPlayerInfo(client_socket, lobby.getPlayer1Username(), lobby.getPlayer2Username(), lobby.getStatus(), lobby);
      sendReconnectInfo(connectedUser == 1 ? lobby.getPlayerSocket2() : lobby.getPlayerSocket1());
      return 0;
}

void startGame(int lobbyIndex) {
      std::lock_guard<std::mutex> lock(lobbies_mutex);
      if(lobbyIndex < 0 || lobbyIndex >= LOBBY_COUNT) return;

      Lobby &lobby = lobbies[lobbyIndex];
      lobby.setStatus(1); 

      std::string p1Name = lobby.getPlayer1Username();
      std::string p2Name = lobby.getPlayer2Username();
      int p1Socket = lobby.getPlayerSocket1();
      int p2Socket = lobby.getPlayerSocket2();

      std::cout << "[LOBBY " << (lobbyIndex+1) << "] Game started between " 
                  << p1Name << " and " << p2Name << std::endl;

      sendStartingPlayerInfo(p1Socket, p1Name, p2Name, 1, lobby); 
      sendStartingPlayerInfo(p2Socket, p1Name, p2Name, 1, lobby);
}