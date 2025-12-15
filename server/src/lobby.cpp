#include "../include/lobby.h"
#include "../include/player.h"
#include "../include/gameLogic.h"
#include <iostream>

#define PLAYER_EMPTY 0
#define PLAYER_ONE 1
#define PLAYER_TWO 2
#define POSSIBLE_MOVE 3

// 1 and 2 will represent players that are playing
#define PAUSE_STATUS 3
#define ENDED_STATUS 0

Lobby::Lobby(int id) {
      this->lobbyId = id;
      this->status = 0;
      this->statusBeforePause = 0;
      this->player1 = nullptr;
      this->player2 = nullptr;

      this->p1WantsRematch = false;
      this->p2WantsRematch = false;
      
      /**for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                  board[i][j] = 0;
            }
      }

      board[3][3] = PLAYER_ONE; // Black
      board[4][4] = PLAYER_ONE; // Black
      board[3][4] = PLAYER_TWO; // White
      board[4][3] = PLAYER_TWO; // White*/

      std::string customState = "3123000022212033221122133111112011222122111112112111111111111123";

      for (int i = 0; i < 64; ++i) {
            int row = i / 8;
            int col = i % 8;
            
            int val = customState[i] - '0';

            if (val == 3) {
                  board[row][col] = 0;
            } else {
                  board[row][col] = val;
            }
      }

      getAvaiableMoves(board, PLAYER_ONE);
}

// GETTERS
int Lobby::getId() const {
      return this->lobbyId;
}

int Lobby::getStatus() const {
      return this->status;
}

int Lobby::getPlayerSocket1() {
      if (this->player1 == nullptr) {
            return -1;
      }   
      return this->player1->socket;
}

int Lobby::getPlayerSocket2() {
      if (this->player2 == nullptr) {
            return -1;
      }
      return this->player2->socket;
}

std::string Lobby::getPlayer1Username() {
      if (this->player1 == nullptr) {
            return "";
      }
      return this->player1->username;
}

std::string Lobby::getPlayer2Username() {
      if (this->player2 == nullptr)
      {
            return "";
      }
      
      return this->player2->username;
}

// SETTERS
void Lobby::setStatus(int newStatus) {
      if (newStatus < 0) return;
      this->status = newStatus;
}

int Lobby::setPlayer(Player* player) {
      if (player == nullptr || player->socket < 0 || player->username.empty()) {
            return -1;
      }
      if (player1 == nullptr) {
            std ::cout << "[LOBBY " << lobbyId << "] Player 1 joined with socket " << player->socket << std::endl;
            player1 = player;
            return 1; // Player 1 joined
      } else if (player2 == nullptr) {
            std ::cout << "[LOBBY " << lobbyId << "] Player 2 joined with socket " << player->socket << std::endl;
            player2 = player;
            return 2; // Player 2 joined
      } else {
            return 3; // Lobby full
      }
}


// RELATED TO RECONNECTION
bool Lobby::isUserConnected(int clientSocket) {
      if (player1 != nullptr && player1->socket == clientSocket || player2 != nullptr && player2->socket == clientSocket) {
            return true;
      }
      return false;
}

int Lobby::reconnectUser(Player new_player) {
      if (player1 != nullptr && player1->username == new_player.username) {
            player1->socket = new_player.socket;
            std::cout << "[LOBBY " << lobbyId << "] Player 1 reconnected with socket " << new_player.socket << std::endl;
            setStatus(statusBeforePause);
            return 1;
      } else if (player2 != nullptr && player2->username == new_player.username) {
            player2->socket = new_player.socket;
            std::cout << "[LOBBY " << lobbyId << "] Player 2 reconnected with socket " << new_player.socket << std::endl;
            setStatus(statusBeforePause);
            return 2;
      }

      return -1;
}

void Lobby::removePlayer(int socket) {
      if(!socket || socket < 0) return;

      p1WantsRematch = false;
      p2WantsRematch = false;

      if (status == 1 || status == 2) {
            statusBeforePause = status;
            status = PAUSE_STATUS;
            
            if (player1 != nullptr && player1->socket == socket) {
                  player1->socket = -1; 
                  std::cout << "[LOBBY " << lobbyId << "] Player 1 disconnected. Game Paused." << std::endl;
            }
            else if (player2 != nullptr && player2->socket == socket) {
                  player2->socket = -1;
                  std::cout << "[LOBBY " << lobbyId << "] Player 2 disconnected. Game Paused." << std::endl;
            }
      }

      else if (status == ENDED_STATUS || status == PAUSE_STATUS) {
            
            if (player1 != nullptr && player1->socket == socket) {
                  player1 = nullptr; // Just detach. If they are offline, socket was -1 anyway.
                  std::cout << "[LOBBY " << lobbyId << "] Player 1 left the lobby." << std::endl;
            }
            else if (player2 != nullptr && player2->socket == socket) {
                  player2 = nullptr;
                  std::cout << "[LOBBY " << lobbyId << "] Player 2 left the lobby." << std::endl;
            }

            // A player is "Gone" if the pointer is null (Empty slot) 
            // OR if the socket is -1 (Zombie/Disconnected player waiting for cleanup)
            bool p1Gone = (player1 == nullptr || player1->socket == -1);
            bool p2Gone = (player2 == nullptr || player2->socket == -1);

            if (p1Gone && p2Gone) {
                  resetLobby();
            }
      }
}


// GAME LOGIC METHODS
std::string Lobby::getBoardStateString() {
      std::string state;
      for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                  state += std::to_string(board[i][j]);
            }
      }

      int score1 = getScoreForPlayer(PLAYER_ONE, board);
      int score2 = getScoreForPlayer(PLAYER_TWO, board);

      state += " " + std::to_string(score1) + " " + std::to_string(score2) + " " + std::to_string(status);
      return state;
}

int Lobby::canUserPlay(int clientSocket) {
      if (player1 != nullptr && player1->socket == clientSocket && status == PLAYER_ONE) {
            return 1;
      } else if (player2 != nullptr && player2->socket == clientSocket && status == PLAYER_TWO) {
            return 2;
      }
      return -1;
}

bool Lobby::validateAndApplyMove(int x, int y, int player) {
      if (board[y][x] != 3 && board[y][x] != 0) return false;

      if (processMove(x, y, board, player, true)) {
            
            int opponent = (player == 1) ? 2 : 1;
            
            // We calculate hints for the OPPONENT now.
            if (getAvaiableMoves(board, opponent)) {
                  setStatus(opponent);
            } 
            else {
                  // Opponent Cannot Play (PASS TURN)
                  std::cout << "[LOBBY " << lobbyId << "] Opponent " << opponent << " has no moves. Checking original player..." << std::endl;

                  if (getAvaiableMoves(board, player)) {
                        setStatus(player); 
                        std::cout << "[LOBBY " << lobbyId << "] Turn passed back to Player " << player << std::endl;
                  } 
                  else {
                        // Neither can play (GAME OVER) ---
                        std::cout << "[LOBBY " << lobbyId << "] No moves possible for anyone. GAME OVER." << std::endl;
                        setStatus(ENDED_STATUS);
                  }
            }
            
            return true;
      }
      return false;
}

int Lobby::calculateWinner() {
      if(status != ENDED_STATUS) {
            return -1;
      }

      return(getWinnerResults(board));
}

void Lobby::resetLobby() {
    std::cout << "[LOBBY " << lobbyId << "] Resetting lobby and cleaning up memory." << std::endl;

    status = ENDED_STATUS;
    statusBeforePause = ENDED_STATUS;

    // --- MEMORY CLEANUP  ---
    if (player1 != nullptr) {
        // If socket is -1, they are a Zombie (Disconnected). We MUST delete them.
        if (player1->socket == -1) {
            delete player1;
            std::cout << "[LOBBY " << lobbyId << "] Player 1 (Zombie) deleted from memory." << std::endl;
        } else {
            std::cout << "[LOBBY " << lobbyId << "] Player 1 detached safely." << std::endl;
        }
        player1 = nullptr;
    }

    if (player2 != nullptr) {
        if (player2->socket == -1) {
            delete player2;
            std::cout << "[LOBBY " << lobbyId << "] Player 2 (Zombie) deleted from memory." << std::endl;
        } else {
            std::cout << "[LOBBY " << lobbyId << "] Player 2 detached safely." << std::endl;
        }
        player2 = nullptr;
    }

    std::string customState = "3123000022212033221122133111112011222122111112112111111111111123";

    for (int i = 0; i < 64; ++i) {
        int row = i / 8;
        int col = i % 8;
        
        int val = customState[i] - '0';

        if (val == 3) {
            board[row][col] = 0;
        } else {
            board[row][col] = val;
        }
    }

    getAvaiableMoves(board, PLAYER_ONE);
}

void Lobby::setRematch(int playerSocket) {
    if (playerSocket == player1->socket) p1WantsRematch = true;
    if (playerSocket == player2->socket) p2WantsRematch = true;
}

void Lobby::restartGame() {
    std::cout << "[LOBBY " << lobbyId << "] Restarting game (Rematch)." << std::endl;

    p1WantsRematch = false;
    p2WantsRematch = false;
    status = 1; // Set back to Active Game

    // Clear Board
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = 0;
        }
    }

    // Re-initialize Pieces (Standard or Custom State)
    board[3][3] = PLAYER_ONE;
    board[4][4] = PLAYER_ONE;
    board[3][4] = PLAYER_TWO;
    board[4][3] = PLAYER_TWO;

    getAvaiableMoves(board, PLAYER_ONE);
}