#pragma once
#include <string>

enum ClientState {
    STATE_LOGIN,     // Connected, but no username yet
    STATE_MENU,      // Logged in, browsing lobbies
    STATE_WAITING,   // Inside a lobby, alone
    STATE_PLAYING,   // Inside a lobby, game is active
    STATE_GAME_OVER  // Game finished
};

class Player {
public:
    int socket;
    std::string username;
    int tolerance; 
    
    ClientState state; 

    Player(int s) : socket(s), tolerance(0), state(STATE_MENU) {}

    void appendName(std::string name) { username = name; }
};