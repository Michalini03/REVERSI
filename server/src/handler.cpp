#include "../include/handler.h"
#include "../include/sender.h" // Handler needs to send responses
#include "../include/global.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <iterator>
#include <sys/socket.h>

// Note: Globals (lobbies, mutexes) are extern'd in globals.h
std::vector<std::string> splitMessage(const std::string &s) {
    std::istringstream iss(s);
    return std::vector<std::string>{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};
}

void handleMessage(int clientSocket, const char* message, Player& player) {
    std::string messageStr(message);   
    std::vector<std::string> args = splitMessage(messageStr);

    if (args.empty() || args[0] != "REV") {
        player.tolerance++;
        std::cout << "[SECURITY] Invalid prefix from client " << clientSocket << std::endl;
        return;
    }

    if (args.size() < 2) {
        std::cout << "[SECURITY] Empty command from client " << clientSocket << std::endl;
        return;
    }

    std::string command = args[1];

    try {
        if (command == "CREATE") {
            // REV CREATE <Username> (Size 3)
            if (args.size() != 3) throw std::runtime_error("Invalid CREATE args");
            
            std::string username = args[2];

            player.appendName(username);
            std::cout << "Processing CREATE command." << std::endl;

            {
                std::lock_guard<std::mutex> lock(lobbies_mutex);
                for (auto &lobby : lobbies) {
                    int connectedUser = lobby.reconnectUser(player);
                    if (connectedUser != -1) {
                        handleReconecting(clientSocket, player, lobby, connectedUser);
                        return;
                    }
                }
            }
            if(sendLobbyList(clientSocket) != 0) throw std::runtime_error("Error when sending Lobby List");
        }
        else if (command == "JOIN") {
            // REV JOIN <LobbyID> (Size 3)
            if (args.size() != 3) throw std::runtime_error("Invalid JOIN args");

            int lobbyId = std::stoi(args[2]); 

            std::cout << "Processing JOIN command for lobby " << lobbyId << std::endl;
            int result = handleLobbyJoin(clientSocket, lobbyId, player);
            
            if(result == 1) {
                std::cout << "[SERVER] Client joined as Player 1 in lobby " << lobbyId << std::endl;
            } else if(result == 2) { 
                std::cout << "[SERVER] Client joined as Player 2 in lobby " << lobbyId << std::endl;
                startGame(lobbyId);
            } else if(result == 3) {
                 std::cout << "[SERVER] Lobby " << lobbyId << " is full." << std::endl;
            } else {
                throw std::runtime_error("Error when joining players to lobby");
            }
        }
        else if (command == "EXIT") {
            // REV EXIT <LobbyID> (Size 3)
            if (args.size() != 3) throw std::runtime_error("Invalid EXIT args");

            int lobbyId = std::stoi(args[2]);
            std::cout << "Processing EXIT command for lobby " << lobbyId << std::endl;

            handleLobbyExit(clientSocket, lobbyId);
        }
        else if (command == "MOVE") {
            // REV MOVE <X> <Y> <LobbyID> (Size 5)
            if (args.size() != 5) throw std::runtime_error("Invalid MOVE args");

            int x = std::stoi(args[2]);
            int y = std::stoi(args[3]);
            int lobbyId = std::stoi(args[4]);

            std::cout << "Processing MOVE command to (" << x << ", " << y << ")" << std::endl;


            if(handleMoving(x, y, clientSocket, lobbyId) != 0) throw std::runtime_error("Error occured while applying move");
        }
        else if (command == "REMATCH") {
            // REV REMATCH <LobbyID> (Size 3)
            if (args.size() != 3) throw std::runtime_error("Invalid REMATCH args");

            int lobbyId = std::stoi(args[2]);
            // TODO: Implement Rematch Logic
        }
        else {
            std::cout << "[SECURITY] Unknown command: " << command << std::endl;
        }

    } catch (const std::exception& e) {
        // This catches wrong argument counts AND non-integer inputs (std::stoi errors)
        std::cerr << "[SECURITY] Malformed command from client " << clientSocket << ": " << e.what() << std::endl;  
    }
}

int handleLobbyJoin(int clientSocket, int lobbyId, Player& player) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) {
        return -1; 
    }

    std::lock_guard<std::mutex> lock(lobbies_mutex);
    for (auto &lobby : lobbies) {
        if (lobby.isUserConnected(clientSocket)) {
            std::cout << "[SERVER] User already connected to a lobby." << std::endl;
            return -1;
        }
    }
    
    Lobby &lobby = lobbies[lobbyId];
    int result = lobby.setPlayer(&player);

    if(result > 0) {
        sendConnectInfo(clientSocket, result);
    }
    
    return result;
}

int handleLobbyExit(int clientSocket, int lobbyId) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) return -1;
    if (clientSocket < 0) return -1;

    std::lock_guard<std::mutex> lock(lobbies_mutex);
    Lobby &lobby = lobbies[lobbyId];
    lobby.removePlayer(clientSocket);
    return 0;
}

int handleMoving(int x, int y, int clientSocket, int lobbyId) {
      if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) return -1;
      if (x < 0 || x >=8 || y < 0 || y >=8) return -1; 
      if (clientSocket < 0) return -1; 
      
      std::lock_guard<std::mutex> lock(lobbies_mutex);
      Lobby &lobby = lobbies[lobbyId];

      int current_player = lobby.canUserPlay(clientSocket);
      if(current_player == -1) {
            std::cout << "[LOBBY " << lobbyId << "] It's not the player's turn or player not found." << std::endl;
            return -1;
      }

      if (lobby.validateAndApplyMove(x, y, current_player)) {
            int clientSocket_1 = lobby.getPlayerSocket1();
            int clientSocket_2 = lobby.getPlayerSocket2();

            sendState(clientSocket_1, lobby);
            sendState(clientSocket_2, lobby);

            int newStatus = lobby.getStatus();

            if (newStatus == ENDED_STATUS) {
                  std::string msg = "REV END " + std::to_string(lobby.calculateWinner()) + "\n";
                  send(clientSocket_1, msg.c_str(), msg.size(), 0);
                  send(clientSocket_2, msg.c_str(), msg.size(), 0);
            }
            else if (newStatus == current_player) {
                  std::string msg = "REV PASS\n"; 
                  send(clientSocket_1, msg.c_str(), msg.size(), 0);
                  send(clientSocket_2, msg.c_str(), msg.size(), 0);
            }
            
            return 0;
      }
      else {
            return 0;
      }
}

int handleReconecting(int clientSocket, Player& player, Lobby& lobby, int connectedUser) {
      if(clientSocket < 0 || &player == nullptr || &lobby == nullptr) {
            return -1;
      }

      sendStartingPlayerInfo(clientSocket, lobby.getPlayer1Username(), lobby.getPlayer2Username(), lobby.getStatus(), lobby);
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