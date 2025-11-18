#include "../include/lobby.h"


#define PLAYER_EMPTY 0
#define PLAYER_ONE 1
#define PLAYER_TWO 2
#define POSSIBLE_MOVE 3

// 1 and 2 will represent players that are playing
#define PAUSE_STATUS -1
#define ENDED_STATUS 0

Lobby::Lobby(int id) {
      this->lobbyId = id;
      this->status = 0;
      this->playerSocket1 = -1;
      this->playerSocket2 = -1;
      
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

int Lobby::appendPlayer(int socket) {
      if (socket < 0) {
            return -1;
      }
      if (playerSocket1 == -1) {
            playerSocket1 = socket;
            return 1; // Player 1 joined
      } else if (playerSocket2 == -1) {
            playerSocket2 = socket;
            return 2; // Player 2 joined
      } else {
            return 3; // Lobby full
      }
}