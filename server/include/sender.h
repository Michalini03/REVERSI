#pragma once
#include <string>
#include "../include/lobby.h"

/**
 * @brief Sends a connection confirmation to the client.
 * * Sends a "REV CONNECT <PlayerNum>" message telling the client
 * if they are Player 1 or Player 2 in the lobby.
 * * @param client_socket The socket descriptor of the target client.
 * @param playerNumber The assigned player number (1 or 2).
 * @return 0 on success, -1 on failure.
 */
int sendConnectInfo(int client_socket, int playerNumber);

/**
 * @brief Notifies a client that their opponent has disconnected.
 * * Sends a "REV DISCONNECT <WhoDisconnected>" message.
 * * @param client_socket The socket descriptor of the remaining player.
 * @param disconnectedUser The player number (1 or 2) who left.
 * @return 0 on success, -1 on failure.
 */
int sendDisconnectInfo(int client_socket, int disconnectedUser);

/**
 * @brief Notifies a client that their opponent has reconnected.
 * * Sends a "REV RECONNECT" message, usually triggering the client
 * to hide any "Waiting for opponent" modals.
 * * @param client_socket The socket descriptor of the waiting player.
 * @return 0 on success, -1 on failure.
 */
int sendReconnectInfo(int client_socket);

/**
 * @brief Sends the Game Start signal and initial data.
 * * Sends "REV START <PlayerNum> <P1Name> <P2Name> <LobbyID>" followed
 * by the initial board state.
 * * @param client_socket The socket descriptor of the target client.
 * @param player1 Username of Player 1.
 * @param player2 Username of Player 2.
 * @param playerNumber The receiving client's player number (1 or 2).
 * @param lobby Reference to the game lobby.
 * @return 0 on success, -1 on failure.
 */
int sendStartingPlayerInfo(int client_socket, std::string player1, std::string player2, int playerNumber, Lobby& lobby);

/**
 * @brief Sends the current board state and scores.
 * * Sends "REV STATE <BoardString> <Score1> <Score2>".
 * Used to synchronize the visual board after every move.
 * * @param client_socket The socket descriptor of the target client.
 * @param lobby Reference to the lobby containing the current board.
 * @return 0 on success, -1 on failure.
 */
int sendState(int client_socket, Lobby& lobby);

/**
 * @brief Sends the list of available lobbies to a client.
 * * Sends "REV LOBBY <Count>". Currently sends the total count,
 * allowing the client to generate the list UI.
 * * @param client_socket The socket descriptor of the target client.
 * @return 0 on success.
 */
int sendLobbyList(int client_socket);