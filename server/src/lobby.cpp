#include "../include/lobby.h"
#include "../include/player.h"
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
      this->player1 = nullptr;
      this->player2 = nullptr;
      
      for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                  board[i][j] = 0;
            }
      }
}

int Lobby::getId() const {
      return this->lobbyId;
}

int Lobby::getStatus() const {
      return this->status;
}

void Lobby::setStatus(int newStatus) {
      if (newStatus < 0) return;
      this->status = newStatus;
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



int Lobby::appendPlayer(Player* player) {
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
            return lobbyId;
      } else if (player2 != nullptr && player2->username == new_player.username) {
            player2->socket = new_player.socket;
            std::cout << "[LOBBY " << lobbyId << "] Player 2 reconnected with socket " << new_player.socket << std::endl;
            return lobbyId;
      }

      return -1;
}

void Lobby::removePlayer(int socket) {
      if (status == ENDED_STATUS) {
            if (player1 != nullptr && player1->socket == socket) {
                  delete player1;
                  player1 = nullptr;
                  std::cout << "[LOBBY " << lobbyId << "] Player 1 disconnected." << std::endl;
            }
            else if (player2 != nullptr && player2->socket == socket) {
                  delete player2;
                  player2 = nullptr;
                  std::cout << "[LOBBY " << lobbyId << "] Player 2 disconnected." << std::endl;
            }
      }

      else if (status == 1 || status == 2) {
            status = PAUSE_STATUS;
            std::cout << "[LOBBY " << lobbyId << "] Game paused due to player disconnection." << std::endl;
            if (player1->socket == socket) {
                  player1->socket = -1;
                  std::cout << "[LOBBY " << lobbyId << " ERROR] Player 1 disconnected." << std::endl;
            }
            else if (player2->socket == socket) {
                  player2->socket = -1;
                  std::cout << "[LOBBY " << lobbyId << " ERROR] Player 2 disconnected." << std::endl;
            }
      }
}