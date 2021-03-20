#include <stdint.h>

#include "eval.h"
#include "main.h"
#include "moveHandler.h"

uint8_t const blackPawnWeights[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    40, 40, 40, 40, 40, 40, 40, 40,
    30, 30, 30, 30, 30, 30, 30, 30,
    20, 20, 20, 20, 20, 20, 20, 20,
    10, 10, 10, 10, 10, 10, 10, 10,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
};

uint8_t const whitePawnWeights[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 10, 10, 10, 10, 10, 10,
    20, 20, 20, 20, 20, 20, 20, 20,
    30, 30, 30, 30, 30, 30, 30, 30,
    40, 40, 40, 40, 40, 40, 40, 40,
    50, 50, 50, 50, 50, 50, 50, 50,
     0,  0,  0,  0,  0,  0,  0,  0,
};

uint8_t const whiteKingWeights[64] = {
     65, 60, 55, 50, 50, 55, 60, 65,
     10, 10, 10, 20, 20, 10, 10, 10,
      0,  0,  0, 10, 10,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
};

uint8_t const blackKingWeights[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0, 10, 10,  0,  0,  0,
     10, 10, 10, 20, 20, 10, 10, 10,
     65, 60, 55, 50, 50, 55, 60, 65,
};

uint8_t const knightWeights[64] = {
      0,  0,  0,  0,  0,   0,  0,  0,
      0,  5, 10, 10, 10,  10,  5,  0,
      5, 10, 40, 40, 40,  40, 10,  5,
      5, 10, 40, 60, 60,  40, 10,  5,
      5, 10, 40, 60, 60,  40, 10,  5,
      5, 10, 40, 40, 40,  40, 10,  5,
      0,  5, 10, 20, 20,  10,  5,  0,
      0,  0,  0,  0,  0,   0,  0,  0,
};

uint8_t const blackBishopWeights[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0, 40, 30, 30, 40,  0,  0,
      0, 50, 50, 20, 20, 50, 50,  0,
     10, 60, 40, 20, 20, 40, 60, 10,
      0, 60, 40,  0,  0, 40, 60,  0,
};

uint8_t const whiteBishopWeights[64] = {
      0, 60, 40,  0,  0, 40, 60,  0,
     10, 60, 40, 20, 20, 40, 60, 10,
      0, 50, 50, 20, 20, 50, 50,  0,
      0,  0, 40, 30, 30, 40,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
};

evaluation_t evaluateBoard(Board *p_board, uint8_t noLegalMoves)
{

    if(noLegalMoves)
    {
        if(p_board->turn == WHITE)
        {
            if(isChecked(p_board, p_board->whiteKing))
            {
                return MATE;
            }
        }

        if(p_board->turn == BLACK)
        {
            if(isChecked(p_board, p_board->blackKing))
            {
                return -MATE;
            }
        }

        return STALEMATE;
    }


    // Material count 
    evaluation_t eval = 0;
    for (uint8_t square = 0; square < 64; square++)
    {
        evaluation_t pieceValue = 0;
        switch (p_board->board[square] & TYPE_MASK)
        {
        case PAWN:
            pieceValue = PAWN_VALUE + (p_board->board[square] & COLOR_MASK == WHITE ? whitePawnWeights[square] : blackPawnWeights[square]);
            break;
        case ROOK:
            pieceValue = ROOK_VALUE;
            break;
        case KNIGHT:
            pieceValue = KNIGHT_VALUE + knightWeights[square];
            break;
        case BISHOP:
            pieceValue = BISHOP_VALUE + (p_board->board[square] & COLOR_MASK == WHITE ? whiteBishopWeights[square] : blackBishopWeights[square]);
            break;
        case QUEEN:
            pieceValue = QUEEN_VALUE;
            break;
        case KING:
            pieceValue = (p_board->board[square] & COLOR_MASK == WHITE ? whiteKingWeights[square] : blackKingWeights[square]);
        default:
            break;
        }

        // Select the color of the computer
        eval += (p_board->board[square] & COLOR_MASK) == BLACK ? pieceValue : -pieceValue;
    }

    return eval;
}