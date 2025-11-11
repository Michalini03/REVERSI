#ifndef SERVER_H
#define SERVER_H

/**
 * @brief Starts the TCP server.
 * * This function initializes the socket, binds it to a port,
 * listens for connections, and enters an infinite loop
 * to accept and handle clients one by one.
 */
void startServer();

#endif // SERVER_H