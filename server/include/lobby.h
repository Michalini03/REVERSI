#pragma once
#include "../include/player.h"
#include <string>


class Lobby {
public:
      Lobby(int id);
      ~Lobby() = default;

      Player* player1;
      Player* player2;
      int getId() const;
      int getStatus() const;
      void setStatus(int newStatus);
      int appendPlayer(Player* player);
      int getPlayerSocket1();
      int getPlayerSocket2();
      std::string getPlayer1Username();
      std::string getPlayer2Username();
      bool isUserConnected(int client_socket);
      int reconnectUser(Player new_player);
      void removePlayer(int socket);
private:
      int lobbyId;
      int status;
      int board[8][8];
};
