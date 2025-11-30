#include "../include/gameLogic.h"
#include <array>
#include <vector>
#include <algorithm>

int getScoreForPlayer(int playerId, int board[][8]) {
    int count = 0;
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            if(board[i][j] == playerId) {
                count++;
            }
        }
    }
    return count; // Example: score is 10 times the player ID
}

void getAvaiableMoves(int board[][8], int currentPlayer) {
    // Placeholder logic for getting available moves
    // In a real implementation, this would analyze the game board
    // and return a list of valid moves
     bool user_can_play = false;
      for(int i = 0; i < 8; ++i) {
            for(int j = 0; j < 8; ++j) {
                  if(board[i][j] == 0 || board[i][j] == 3) {
                  bool is_valid = validateMove(i, j, board, currentPlayer);
                  if(is_valid) {
                        // Store or print the valid move (i, j)
                              board[i][j] = 3; // Mark as possible move
                              user_can_play = true;
                  }
                        else {
                              board[i][j] = 0; // Mark as empty
                        }
                  }
            }
      }
}

bool validateMove(int x, int y, int board[][8], int currentPlayer) {
      // Placeholder logic for validating a move
      // In a real implementation, this would check the game rules
      if (x < 0 || x >= 8 || y < 0 || y >= 8) {
            return false; // Move is out of bounds
      }

      if (board[y][x] == 1 || board[y][x] == 2) {
            return false; // Cell is not empty
      }

      bool valid_move = false;
      int color = currentPlayer;
      int opponent = (color == 1) ? 2 : 1;

      std::vector<std::array<int,2>> directions = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
      };

      for_each(directions.begin(), directions.end(), [&](const std::array<int,2>& dir) {
            int dx = dir[0];
            int dy = dir[1];

            int size = 0;
            int currX = x + dx;
            int currY = y + dy;
            bool found_opponent = false;

            while (currX >= 0 && currX < 8 && currY >= 0 && currY < 8) {
                  if (board[currY][currX] == opponent) {
                        found_opponent = true;
                        size++;
                        currX += dx;
                        currY += dy;
                  } else if (board[currY][currX] == color) {
                        if (found_opponent) {
                              valid_move = true;
                        }
                        break;
                  } 
                  else {
                        break;
                  }
            }
      });
      return valid_move; // Move is valid
}