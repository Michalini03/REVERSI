#pragma once
#include "../include/player.h"
#include "../include/lobby.h"
#include <cstring>
#include <vector>

/**
 * @brief Parse message to an array of strings, made for
 * argument counting
 * @return Array of params
 */
std::vector<std::string> splitMessage(const std::string &s);

/**
 * @brief Parses and routes an incoming raw message.
 * * Decodes the message string (e.g., "REV MOVE 3 4 0") and calls
 * the appropriate specific handler (handleMoving, handleLobbyJoin, etc.).
 * * @param clientSocket The socket of the client sending the message.
 * @param message The raw C-string message received from the TCP buffer.
 * @param player Reference to the Player object associated with this socket.
 */
void handleMessage(int clientSocket, const char* message, Player& player);

/**
 * @brief Logic for a player attempting to join a specific lobby.
 * * Checks if the lobby is full, assigns the player to a slot (P1/P2),
 * and sends the corresponding connection info back to the client.
 * * @param clientSocket The socket of the joining player.
 * @param lobbyId The ID of the target lobby (0-indexed).
 * @param player Reference to the Player object.
 * @return 1 if joined as P1, 2 if joined as P2, 3 if full, -1 on error.
 */
int handleLobbyJoin(int clientSocket, int lobbyId, Player& player);

/**
 * @brief Logic for a player leaving a lobby.
 * * Removes the player from the lobby. If a game was in progress,
 * it triggers the Pause/Disconnect logic for the opponent.
 * * @param clientSocket The socket of the leaving player.
 * @param lobbyId The ID of the lobby.
 * @return 0 on success, -1 on error.
 */
int handleLobbyExit(int clientSocket, int lobbyId);

/**
 * @brief Processes a game move request.
 * * Validates the move, updates the board, switches turns, and broadcasts
 * the new state to both players. also checks for Game Over or Pass conditions.
 * * @param x Column index of the move (0-7).
 * @param y Row index of the move (0-7).
 * @param clientSocket The socket of the player making the move.
 * @param lobbyId The ID of the lobby where the game is happening.
 * @return 0 on success, -1 on invalid move/error.
 */
int handleMoving(int x, int y, int clientSocket, int lobbyId);

/**
 * @brief Logic for restoring a player's session after a disconnect.
 * * Re-associates the new socket with the old player slot in the lobby
 * and syncs the game state so the match can resume.
 * * @param clientSocket The new socket descriptor.
 * @param player Reference to the new Player object.
 * @param lobby Reference to the lobby the user is rejoining.
 * @param connectedUser The player number (1 or 2) they are reconnecting as.
 * @return 0 on success, -1 on error.
 */
int handleReconecting(int clientSocket, Player& player, Lobby& lobby, int connectedUser);

int handleRematch(int clientSocket, int lobbyId);

/**
 * @brief Starts the match in a specific lobby.
 * * Sets the lobby status to Active and sends the "REV START" command
 * to both connected players.
 * * @param lobbyIndex The ID of the lobby to start.
 */
void startGame(int lobbyIndex);