#pragma once

int getScoreForPlayer(int userID, int board[][8]);

void getAvaiableMoves(int board[][8], int currentPlayer);

bool validateMove(int x, int y, int board[][8], int currentPlayer);


