#include "../include/sender.h"
#include "../include/global.h"
#include <iostream>
#include <sys/socket.h>
#include <string>

int sendConnectInfo(int client_socket, int playerNumber) {
    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " CONNECT " + std::to_string(playerNumber) + "\n";
    
    send(client_socket, message.c_str(), message.size(), 0);
    return 0;
}

int sendDisconnectInfo(int client_socket, int disconnectedUser) {
    if(client_socket == -1 || disconnectedUser == -1) {
        return -1;
    }

    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " DISCONNECT " + std::to_string(disconnectedUser) + "\n";

    send(client_socket, message.c_str(), message.size(), 0);
    return 0;
}

int sendReconnectInfo(int client_socket) {
    if(client_socket < 0) {
        return -1;
    }

    std::string prefix(PREFIX_GAME);
    std::string message = prefix + " RECONNECT" + "\n";

    send(client_socket, message.c_str(), message.size(), 0);
    return 0;
}

int sendStartingPlayerInfo(int client_socket, std::string player1, std::string player2, int playerNumber, Lobby& lobby) {
    std::string prefix(PREFIX_GAME);
    std::cout << "Sending state" << std::endl;
    // Send START message
    std::string message = prefix + " START " + std::to_string(playerNumber);
    message += " " + player1 + " " + player2 + " " + std::to_string(lobby.getId());
    message += "\n";
    send(client_socket, message.c_str(), message.size(), 0);

    sendState(client_socket, lobby);
    return 0;
}

int sendState(int client_socket, Lobby& lobby) {
    std::string prefix(PREFIX_GAME);

    if (client_socket < 0 || !&lobby) {
        return -1;
    }

    std::string boardState = prefix + " STATE " + lobby.getBoardStateString() + "\n";
    send(client_socket, boardState.c_str(), boardState.size(), 0);
    return 0;
}

int sendLobbyList(int client_socket) {
    std::string prefix(PREFIX_GAME);
    std::string lobbyList = prefix + " LOBBY " + std::to_string(LOBBY_COUNT) + "\n";
    
    send(client_socket, lobbyList.c_str(), lobbyList.size(), 0);
    return 0;
}