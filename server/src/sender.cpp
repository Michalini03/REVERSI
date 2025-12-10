#include "../include/sender.h"
#include "../include/global.h"
#include <iostream>
#include <sys/socket.h>
#include <string>

int sendConnectInfo(int clientSocket, int playerNumber) {
    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " CONNECT " + std::to_string(playerNumber) + "\n";
    
    send(clientSocket, message.c_str(), message.size(), 0);
    return 0;
}

int sendDisconnectInfo(int clientSocket, int disconnectedUser) {
    if(clientSocket == -1 || disconnectedUser == -1) {
        return -1;
    }

    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " DISCONNECT " + std::to_string(disconnectedUser) + "\n";

    send(clientSocket, message.c_str(), message.size(), 0);
    return 0;
}

int sendReconnectInfo(int clientSocket) {
    if(clientSocket < 0) {
        return -1;
    }

    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " RECONNECT" + "\n";

    send(clientSocket, message.c_str(), message.size(), 0);
    return 0;
}

int sendStartingPlayerInfo(int clientSocket, std::string player1, std::string player2, int playerNumber, Lobby& lobby) {
    std::string prefix(PREFIX_GAME);
    std::cout << "Sending state" << std::endl;
    // Send START message
    std::string message = prefix + " START " + std::to_string(playerNumber);
    message += " " + player1 + " " + player2 + " " + std::to_string(lobby.getId());
    message += "\n";
    send(clientSocket, message.c_str(), message.size(), 0);

    sendState(clientSocket, lobby);
    return 0;
}

int sendState(int clientSocket, Lobby& lobby) {
    std::string prefix(PREFIX_GAME);

    if (clientSocket < 0 || !&lobby) {
        return -1;
    }

    std::string boardState = prefix + " STATE " + lobby.getBoardStateString() + "\n";
    send(clientSocket, boardState.c_str(), boardState.size(), 0);
    return 0;
}

int sendLobbyList(int clientSocket) {
    if(!clientSocket || clientSocket < 0) return 1;

    std::string prefix(PREFIX_GAME);
    std::string lobbyList = prefix + " LOBBY " + std::to_string(LOBBY_COUNT) + "\n";
    
    send(clientSocket, lobbyList.c_str(), lobbyList.size(), 0);
    return 0;
}