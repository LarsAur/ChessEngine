#include <stdint.h>
#include <stdio.h>

#include "eval.h"
#include "main.h"
#include "moveHandler.h"

#define PAWN_PHASE 0
#define KNIGHT_PHASE 1
#define BISHOP_PHASE 1
#define ROOK_PHASE 2
#define QUEEN_PHASE 4
#define TOTAL_PHASE PAWN_PHASE * 16 + KNIGHT_PHASE * 4 + BISHOP_PHASE * 4 + ROOK_PHASE * 4 + QUEEN_PHASE * 2

uint8_t getPhase(Board *p_board);

uint8_t const blackPawnWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,	
    50,	50,	50,	50,	50,	50,	50,	50,
    40,	40,	40,	40,	40,	40,	40,	40,
    30,	30,	30,	30,	30,	30,	30,	30,
    20,	20,	20,	20,	20,	20,	20,	20,
    10,	10,	10,	10,	10,	10,	10,	10,	
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
};

uint8_t const whitePawnWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	
    10,	10,	10,	10,	10,	10,	10,	10,	
    20,	20,	20,	20,	20,	20,	20,	20,	
    30,	30,	30,	30,	30,	30,	30,	30,	
    40,	40,	40,	40,	40,	40,	40,	40,	
    50,	50,	50,	50,	50,	50,	50,	50,	
    0,	0,	0,	0,	0,	0,	0,	0,
};

uint8_t const whiteKingWeights[64] = {
    65,	60,	55,	50,	50,	55,	60,	65,
    10,	10,	10,	20,	20,	10,	10,	10,	
    0,	0,	0,	10,	10,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,
};

uint8_t const blackKingWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	10,	10,	0,	0,	0,
    10,	10,	10,	20,	20,	10,	10,	10,
    65,	60,	55,	50,	50,	55,	60,	65,
};

uint8_t const knightWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	5,	10,	10,	10,	10,	5,	0,
    5,	10,	40,	40,	40,	40,	10,	5,
    5,	10,	40,	60,	60,	40,	10,	5,
    5,	10,	40,	60,	60,	40,	10,	5,	
    5,	10,	40,	40,	40,	40,	10,	5,	
    0,	5,	10,	20,	20,	10,	5,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
};

uint8_t const blackBishopWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	40,	30,	30,	40,	0,	0,
    0,	50,	50,	20,	20,	50,	50,	0,
    10,	60,	40,	20,	20,	40,	60,	10,
    0,	60,	40,	0,	0,	40,	60,	0,
};

uint8_t const whiteBishopWeights[64] = {
    0,	60,	40,	0,	0,	40,	60,	0,	
    10,	60,	40,	20,	20,	40,	60,	10,	
    0,	50,	50,	20,	20,	50,	50,	0,	
    0,	0,	40,	30,	30,	40,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,
};

uint8_t const kingEndgameWeights[64] = {
    0,   10, 20, 30, 30, 20, 10,  0,
    10,  50, 50, 50, 50, 50, 50, 10,
    20,  50, 100,100,100,100,50, 20,
    30,  50, 100,150,150,100,50, 30,
    30,  50, 100,150,150,100,50, 30,
    20,  50, 100,100,100,100,50, 20,
    10,  50, 50, 50, 50, 50, 50, 10,
    0,   10, 20, 30, 30, 20, 10,  0,
};

evaluation_t evaluateBoard(Board *p_board, uint8_t noLegalMoves, uint8_t color)
{
    // Evaluate terminal position
    if (noLegalMoves == NO_LEGAL_MOVES)
    {
        if (p_board->turn == WHITE)
        {
            if (isChecked(p_board, WHITE))
            {
                if(color == BLACK)
                {
                    return MATE;
                }

                return -MATE;
            }
        }

        if (p_board->turn == BLACK)
        {
            if (isChecked(p_board, BLACK))
            {
                if(color == BLACK)
                {
                    return -MATE;
                }
                return MATE;
            }
        }

        return STALEMATE;
    }

    // Material count and opening position
    evaluation_t openingEvaluation = 0;
    for (uint8_t square = 0; square < 64; square++)
    {
        evaluation_t pieceValue = 0;
        switch (p_board->board[square] & TYPE_MASK)
        {
        case PAWN:
            pieceValue = PAWN_VALUE + ((p_board->board[square] & COLOR_MASK) == WHITE ? whitePawnWeights[square] : blackPawnWeights[square]);
            break;
        case ROOK:
            pieceValue = ROOK_VALUE;
            break;
        case KNIGHT:
            pieceValue = KNIGHT_VALUE + knightWeights[square];
            break;
        case BISHOP:
            pieceValue = BISHOP_VALUE + ((p_board->board[square] & COLOR_MASK) == WHITE ? whiteBishopWeights[square] : blackBishopWeights[square]);
            break;
        case QUEEN:
            pieceValue = QUEEN_VALUE;
            break;
        case KING:
            pieceValue = ((p_board->board[square] & COLOR_MASK) == WHITE ? whiteKingWeights[square] : blackKingWeights[square]);
        default:
            break;
        }

        // Select the color of the computer
        openingEvaluation += (p_board->board[square] & COLOR_MASK) == BLACK ? pieceValue : -pieceValue;
    }

    evaluation_t endgameEvaluation = 0;
    for (uint8_t square = 0; square < 64; square++)
    {
        evaluation_t pieceValue = 0;
        switch (p_board->board[square] & TYPE_MASK)
        {
        case PAWN:
            pieceValue = PAWN_VALUE + ((p_board->board[square] & COLOR_MASK) == WHITE ? whitePawnWeights[square] : blackPawnWeights[square]);
            break;
        case ROOK:
            pieceValue = ROOK_VALUE;
            break;
        case KNIGHT:
            pieceValue = KNIGHT_VALUE + knightWeights[square];
            break;
        case BISHOP:
            pieceValue = BISHOP_VALUE + ((p_board->board[square] & COLOR_MASK) == WHITE ? whiteBishopWeights[square] : blackBishopWeights[square]);
            break;
        case QUEEN:
            pieceValue = QUEEN_VALUE;
            break;
        case KING:
            pieceValue = kingEndgameWeights[square];
        default:
            break;
        }

        // Select the color of the computer
        endgameEvaluation += (p_board->board[square] & COLOR_MASK) == BLACK ? pieceValue : -pieceValue;
    }

    uint8_t phase = getPhase(p_board);

    evaluation_t eval = phase * endgameEvaluation + (TOTAL_PHASE - phase) * openingEvaluation;

    if (color == BLACK)
    {
        return eval;
    }

    return -eval;
}

const uint8_t typeToPhase[7] =
{
    0,   // Empty
    PAWN_PHASE, 
    ROOK_PHASE, 
    KNIGHT_PHASE, 
    BISHOP_PHASE, 
    0,   // King
    QUEEN_PHASE, 
};

// Returns the phase of the board as number between TOTAL_PHASE (Endgame) and 0 (Opening)
// TODO: fix for promotion
uint8_t getPhase(Board *p_board)
{
    uint8_t phase = TOTAL_PHASE;
    for (uint8_t square = 0; square < 64; square++)
    {
        phase -= typeToPhase[p_board->board[square] & TYPE_MASK];
    }
    return phase;
}