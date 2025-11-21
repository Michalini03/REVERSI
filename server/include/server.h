#pragma once

/**
 * @brief Starts the TCP server.
 * * This function initializes the socket, binds it to a port,
 * listens for connections, and enters an infinite loop
 * to accept and handle clients one by one.
 */
void startServer();

/**
 * @brief Handles the logic for a connected client.
 * @param client_socket The file descriptor for the client's socket.
 */
void handleClientLogic(int client_socket);

/**
 * @brief Handles an incoming message from a client.
 * @param client_socket The file descriptor for the client's socket.
 * @param message The null-terminated message received from the client.
 */
void handleMessage(int client_socket, const char* message);

/**
 * @brief Sends the list of available lobbies to the client.
 * @param client_socket The file descriptor for the client's socket.
 * @return 0 on success, non-zero on failure.
 */
int sendLobbyList(int client_socket);

int sendConnectInfo(int client_socket, int player_id);

int sendStartingPlayerInfo(int client_socket, int player_id);

/**
 * @brief Joins client socket to lobby or inform user about fully lobby
 * @param lient_socket The file descriptor for the client's socket.
 * @param lobbyId id of lobby (index in vector)
 * @param 1 - user is player1, 2 - user is player2, 3 - lobby is full, 0 - error
*/
int handleLobbyJoin(int client_socket, int lobbyId);