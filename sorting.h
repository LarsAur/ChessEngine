#ifndef SORTING_H

#define SORTING_H

#include "moveHandler.h"
#include "main.h"

void sort(ArrayList *p_list, Board *p_board, uint16_t (*orderEval)(Move *, Board *));

#endif