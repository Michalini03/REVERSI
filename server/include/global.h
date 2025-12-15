#pragma once
#include <vector>
#include <mutex>
#include "../include/lobby.h"

#define PORT 10001
#define LOBBY_COUNT 5
#define PREFIX_GAME "REV"
#define PAUSE_STATUS 3
#define ENDED_STATUS 0

extern std::vector<Lobby> lobbies;
extern std::vector<int> clientSockets;
extern std::mutex clients_mutex;
extern std::mutex lobbies_mutex;