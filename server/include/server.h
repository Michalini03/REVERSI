#pragma once

/**
 * @brief Starts the TCP server.
 * * This function initializes the socket, binds it to a port,
 * listens for connections, and enters an infinite loop
 * to accept and handle clients one by one.
 */
void startServer();

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
