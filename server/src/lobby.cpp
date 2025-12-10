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
bool Lobby::isUserConnected(int client_socket) {
      if (player1 != nullptr && player1->socket == client_socket || player2 != nullptr && player2->socket == client_socket) {
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
      if (status == ENDED_STATUS || status == PAUSE_STATUS) {
            if (player1 != nullptr && player1->socket == socket) {
                  player1->socket = -1;
                  std::cout << "[LOBBY " << lobbyId << "] Player 1 disconnected." << std::endl;
            }
            else if (player2 != nullptr && player2->socket == socket) {
                  player2->socket = -1;
                  std::cout << "[LOBBY " << lobbyId << "] Player 2 disconnected." << std::endl;
            }

            bool p1Gone = (player1 == nullptr || player1->socket == -1);
            
            bool p2Gone = (player2 == nullptr || player2->socket == -1);

            if (p1Gone && p2Gone) {
                  resetLobby();
            }
      }
      else if (status == 1 || status == 2) {
            statusBeforePause = status;
            status = PAUSE_STATUS;
            std::cout << "[LOBBY " << lobbyId << "] Game paused due to player disconnection." << std::endl;
            if (player1->socket == socket) {
                  player1->socket = -1;
                  std::cout << "[LOBBY " << lobbyId << "] Player 1 disconnected." << std::endl;
            }
            else if (player2->socket == socket) {
                  player2->socket = -1;
                  std::cout << "[LOBBY " << lobbyId << "] Player 2 disconnected." << std::endl;
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

      state += " " + std::to_string(score1) + " " + std::to_string(score2);
      return state;
}

int Lobby::canUserPlay(int client_socket) {
      if (player1 != nullptr && player1->socket == client_socket && status == PLAYER_ONE) {
            return 1;
      } else if (player2 != nullptr && player2->socket == client_socket && status == PLAYER_TWO) {
            return 2;
      }
      return -1;
}

bool Lobby::validateAndApplyMove(int x, int y, int player) {
      if (board[y][x] != 3 && board[y][x] != 0) return false;

      if (processMove(x, y, board, player, true)) {
            
            // Define who is who
            int opponent = (player == 1) ? 2 : 1;
            
            // --- STEP A: Can the Opponent play? ---
            // We calculate hints for the OPPONENT now.
            if (getAvaiableMoves(board, opponent)) {
                  // Standard case: Opponent has moves, so we switch turn.
                  setStatus(opponent);
            } 
            else {
                  // --- STEP B: Opponent Cannot Play (PASS TURN) ---
                  std::cout << "[LOBBY] Opponent " << opponent << " has no moves. Checking original player..." << std::endl;

                  // We check if the ORIGINAL player can move again.
                  if (getAvaiableMoves(board, player)) {
                        // Original player goes again (Opponent passes)
                        setStatus(player); 
                        std::cout << "[LOBBY] Turn passed back to Player " << player << std::endl;
                  } 
                  else {
                        // --- STEP C: Neither can play (GAME OVER) ---
                        std::cout << "[LOBBY] No moves possible for anyone. GAME OVER." << std::endl;
                        setStatus(ENDED_STATUS);
                        // Optional: Calculate final winner here if you want to store it
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
      std::cout << "[LOBBY " << lobbyId << "] Resetting lobby" << std::endl;

      status = ENDED_STATUS;
      statusBeforePause = ENDED_STATUS;
      if (player1 != nullptr) {
            player1 = nullptr;
      }
      if (player2 != nullptr) {
            player2 = nullptr;
      }
      
      /*for (int i = 0; i < 8; ++i) {
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