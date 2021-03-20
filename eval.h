#ifndef EVAL_H

#define EVAL_H

#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 300
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 1000

#define STALEMATE 0
#define MATE 9223372036854775807

#include <stdint.h>
#include "main.h"

typedef int64_t evaluation_t;
evaluation_t evaluateBoard(Board *p_board, uint8_t noLegalMoves);

#endif