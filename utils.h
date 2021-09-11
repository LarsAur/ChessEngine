#ifndef UTILS_H

#define UTILS_H

#include "main.h"
#include "moveHandler.h"

void printBoard(Board *p_board);
void printMoveList(ArrayList *p_list);
void printMove(Move *p_move);
Move selectMove(Board *p_board);

#endif // UTILS_H