#pragma once

class Lobby {
public:
    Lobby(int id);
    int getId() const;
    int getStatus() const;
    int appendPlayer(int socket);
private:
    int lobbyId;
    int status;
    int playerSocket1;
    int playerSocket2;
    int board[8][8];
};
