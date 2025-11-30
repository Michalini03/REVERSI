#pragma once

int getScoreForPlayer(int userID, int board[][8]);

bool getAvaiableMoves(int board[][8], int currentPlayer);

bool processMove(int x, int y, int board[][8], int player, bool apply);

bool validateMove(int x, int y, int board[][8], int currentPlayer);


