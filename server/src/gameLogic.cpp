#include "../include/gameLogic.h"
#include <array>
#include <vector>
#include <iostream>
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

bool processMove(int x, int y, int board[][8], int player, bool apply) {
      // 1. Bounds Check
      if (x < 0 || x >= 8 || y < 0 || y >= 8) return false;

      // 2. Occupied Check: You cannot place a piece on top of another player
      // We allow 0 (Empty) and 3 (Hint)
      if (board[y][x] == 1 || board[y][x] == 2) {
            std::cout << "hovno" << std::endl;
            return false; 
      }

      int opponent = (player == 1) ? 2 : 1;
      bool valid_move = false;

      // 8 Directions
      int dx[] = {-1, -1, -1,  0,  0,  1, 1, 1};
      int dy[] = {-1,  0,  1, -1,  1, -1, 0, 1};

      for (int i = 0; i < 8; i++) {
            int currX = x + dx[i];
            int currY = y + dy[i];
            bool found_opponent = false;
            
            // Walk in this direction
            while (currX >= 0 && currX < 8 && currY >= 0 && currY < 8) {
                  
                  if (board[currY][currX] == opponent) {
                  // Found an opponent, keep walking to look for our piece
                  found_opponent = true;
                  currX += dx[i];
                  currY += dy[i];
                  } 
                  else if (board[currY][currX] == player) {
                  // Found our own piece!
                  if (found_opponent) {
                        // SANDWICH CONFIRMED
                        valid_move = true;

                        // Optimization: If we are only checking (hints), we can stop now!
                        if (!apply) return true;

                        // If applying, walk BACKWARDS and flip
                        int flipX = currX - dx[i];
                        int flipY = currY - dy[i];
                        while (flipX != x || flipY != y) {
                              board[flipY][flipX] = player;
                              flipX -= dx[i];
                              flipY -= dy[i];
                        }
                  }
                  break; // Stop looking in this direction (we hit our own piece)
                  } 
                  else {
                  // Hit an Empty cell (0) or Hint (3) -> Sandwich broken
                  break; 
                  }
            }
      }
      
      // 3. Final Placement
      if (valid_move && apply) {
            board[y][x] = player;
            // std::cout << "Move applied at " << x << "," << y << std::endl;
      }

      return valid_move;
}

bool getAvaiableMoves(int board[][8], int currentPlayer) {
      bool can_play = false;

      for(int r = 0; r < 8; ++r) {
           for(int c = 0; c < 8; ++c) {
                if (board[r][c] == 3) board[r][c] = 0;
           }
      }

      // Calculate new hints
      for(int y = 0; y < 8; ++y) { // y is Row
            for(int x = 0; x < 8; ++x) { // x is Column
                  if(board[y][x] == 0) { // Only check empty spots
                       // validateMove expects (x, y)
                       if(validateMove(x, y, board, currentPlayer)) {
                            board[y][x] = 3; 
                            can_play = true;
                       }
                  }
            }
      }

      return can_play;
}

bool validateMove(int x, int y, int board[][8], int currentPlayer) {
    // Pass 'false' to just check validity without modifying board
    return processMove(x, y, board, currentPlayer, false);
}