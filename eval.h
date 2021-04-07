#ifndef EVAL_H

#define EVAL_H

#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 300
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 1000

#define STALEMATE 0
#define MATE 1000000000

#define LEGAL_MOVES_EXIST 0
#define NO_LEGAL_MOVES 1

#include <stdint.h>
#include "main.h"

typedef int64_t evaluation_t;
evaluation_t evaluateBoard(Board *p_board, uint8_t noLegalMoves, uint8_t color);
uint8_t getPhase(Board *p_board);
void initEvalTables();

#endif