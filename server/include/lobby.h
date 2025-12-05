#pragma once
#include "../include/player.h"
#include <string>


class Lobby {
public:
      Lobby(int id);
      ~Lobby() = default;

      Player* player1;
      Player* player2;

      // Getters and Setters
      int getId() const;
      int getStatus() const;
      void setStatus(int newStatus);
      int setPlayer(Player* player);
      int getPlayerSocket1();
      int getPlayerSocket2();
      std::string getPlayer1Username();
      std::string getPlayer2Username();

      // Connection-related methods c
      bool isUserConnected(int client_socket);
      int reconnectUser(Player new_player);
      void removePlayer(int socket);
      void resetLobby();

      // Game-related methods
      int canUserPlay(int client_socket);
      int calculateWinner();
      bool validateAndApplyMove(int x, int y, int client_socket);
      std::string getBoardStateString();
private:
      int statusBeforePause;
      int lobbyId;
      int status;
      int board[8][8];
};
