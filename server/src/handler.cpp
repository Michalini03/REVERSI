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
            handleRematch(clientSocket, lobbyId);
        }
        else if (command == "HEARTBEAT") {
            return;
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

    int result;
    Lobby *lobbyPtr = nullptr;
    {
        std::lock_guard<std::mutex> lock(lobbies_mutex);
        for (auto &lobby : lobbies) {
            if (lobby.isUserConnected(clientSocket)) {
                std::cout << "[SERVER] User already connected to a lobby." << std::endl;
                return -1;
            }
        }

        lobbyPtr = &lobbies[lobbyId];
        result = lobbyPtr->setPlayer(&player);
    }

    if(result > 0) {
        sendConnectInfo(clientSocket, result);
    }
    
    return result;
}

int handleLobbyExit(int clientSocket, int lobbyId) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) return -1;
    if (clientSocket < 0) return -1;

    Lobby *lobby = nullptr;
    int opponentSocket = -1;
    int leaverId = 0;
    
    {
        std::lock_guard<std::mutex> lock(lobbies_mutex);
        lobby = &lobbies[lobbyId];
    
        if (lobby->getPlayerSocket1() == clientSocket) {
            leaverId = 1;
            opponentSocket = lobby->getPlayerSocket2();
        } 
        else if (lobby->getPlayerSocket2() == clientSocket) {
            leaverId = 2;
            opponentSocket = lobby->getPlayerSocket1();
        }
    
        lobby->removePlayer(clientSocket);
    }

    if (opponentSocket != -1) {
        sendDisconnectInfo(opponentSocket, leaverId);
    }
    return 0;
}

int handleMoving(int x, int y, int clientSocket, int lobbyId) {
    if (lobbyId < 0 || lobbyId >= LOBBY_COUNT) return -1;
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return -1;
    if (clientSocket < 0) return -1;

    int clientSocket1 = -1, clientSocket2 = -1;
    bool valid = false;
    std::string boardStateMsg; // We will store the message here safely
    std::string extraMsg;      // For END or PASS
    
    Lobby *lobby = nullptr;

    {
        std::lock_guard<std::mutex> lock(lobbies_mutex);
        lobby = &lobbies[lobbyId];

        int currentPlayer = lobby->canUserPlay(clientSocket);
        if(currentPlayer == -1) {
            std::cout << "[LOBBY " << lobbyId << "] Not player's turn." << std::endl;
            return -1;
        }

        valid = lobby->validateAndApplyMove(x, y, currentPlayer);

        if (valid) {
            clientSocket1 = lobby->getPlayerSocket1();
            clientSocket2 = lobby->getPlayerSocket2();
            int newStatus = lobby->getStatus();


            boardStateMsg = "REV STATE " + lobby->getBoardStateString() + "\n";

            if (newStatus == ENDED_STATUS) {
                extraMsg = "REV END " + std::to_string(lobby->calculateWinner()) + "\n";
            } else if (newStatus == currentPlayer) {
                extraMsg = "REV PASS\n";
            }
        }
    }

    if (valid) {
        if(clientSocket1 != -1) send(clientSocket1, boardStateMsg.c_str(), boardStateMsg.size(), 0);
        if(clientSocket2 != -1) send(clientSocket2, boardStateMsg.c_str(), boardStateMsg.size(), 0);

        if (!extraMsg.empty()) {
            if(clientSocket1 != -1) send(clientSocket1, extraMsg.c_str(), extraMsg.size(), 0);
            if(clientSocket2 != -1) send(clientSocket2, extraMsg.c_str(), extraMsg.size(), 0);
        }
    }
    return 0;
}

int handleReconecting(int clientSocket, Player& player, Lobby& lobby, int connectedUser) {
      if(clientSocket < 0 || &player == nullptr || &lobby == nullptr) {
            return -1;
      }

      sendStartingPlayerInfo(clientSocket, lobby.getPlayer1Username(), lobby.getPlayer2Username(), lobby.getStatus(), lobby);
      sendReconnectInfo(connectedUser == 1 ? lobby.getPlayerSocket2() : lobby.getPlayerSocket1());
      return 0;
}

int handleRematch(int clientSocket, int lobbyId) {
    if(clientSocket < 0 || lobbyId < 0 || lobbyId > LOBBY_COUNT) {
        return -1;
    }

    Lobby *lobby = nullptr;
    int playerSocket1, playerSocket2;
    std::string name1, name2;
    bool wantsRematch = false;

    {
        std::lock_guard<std::mutex> lock(lobbies_mutex);
        lobby = &lobbies[lobbyId];
        lobby->setRematch(clientSocket);

        wantsRematch = lobby->p1WantsRematch && lobby->p2WantsRematch;
    
        if (wantsRematch) {
            lobby->restartGame();
            
            name1 = lobby->getPlayer1Username();
            name2 = lobby->getPlayer2Username();
    
            playerSocket1 = lobby->getPlayerSocket1();
            playerSocket2 = lobby->getPlayerSocket2();
        } 
    }

    if(wantsRematch) {
        sendStartingPlayerInfo(playerSocket1, name1, name2, 1, *lobby);
        sendStartingPlayerInfo(playerSocket2, name1, name2, 1, *lobby);
    }

    return 0;
}


void startGame(int lobbyIndex) {
    if(lobbyIndex < 0 || lobbyIndex >= LOBBY_COUNT) return;

    std::string p1Name, p2Name;
    int p1Socket, p2Socket;
    Lobby* lobbyPtr = nullptr;

    {
        std::lock_guard<std::mutex> lock(lobbies_mutex);

        lobbyPtr = &lobbies[lobbyIndex];
        lobbyPtr->setStatus(1); 

        p1Name = lobbyPtr->getPlayer1Username();
        p2Name = lobbyPtr->getPlayer2Username();
        p1Socket = lobbyPtr->getPlayerSocket1();
        p2Socket = lobbyPtr->getPlayerSocket2();

        std::cout << "[LOBBY " << (lobbyIndex+1) << "] Game started between " 
                << p1Name << " and " << p2Name << std::endl;
    }

    if (lobbyPtr != nullptr) {
        sendStartingPlayerInfo(p1Socket, p1Name, p2Name, 1, *lobbyPtr); 
        sendStartingPlayerInfo(p2Socket, p1Name, p2Name, 1, *lobbyPtr);
    }
}