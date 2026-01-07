#pragma once

/**
 * @brief Initializes and runs the main server loop.
 * * 1. Creates the server socket.
 * 2. Binds to PORT.
 * 3. Listens for incoming connections.
 * 4. Spawns a new thread (handleClientLogic) for each connected client.
 */
void startServer(std::string ip, int port);

/**
 * @brief The main thread function for a single connected client.
 * * Handles the TCP receive loop, buffering data to fix packet coalescing,
 * and managing the lifecycle of the Player object (memory allocation/cleanup).
 * * @param clientSocket The file descriptor for the connected client.
 */
void handleClientLogic(int clientSocket);