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

void m_inversePositionalValues(int8_t src[], int8_t dest[]);

int8_t emptyWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
};

int8_t whitePawnOpeningWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	
    10,	10,	10,	10,	10,	10,	10,	10,	
    20,	20,	20,	20,	20,	20,	20,	20,	
    30,	30,	30,	30,	30,	30,	30,	30,	
    40,	40,	40,	40,	40,	40,	40,	40,	
    50,	50,	50,	50,	50,	50,	50,	50,	
    0,	0,	0,	0,	0,	0,	0,	0,
};

int8_t whiteRookOpeningWeights[64] = {
    0,	10,	20,	30,	30,	20,	10,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,  0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    50,	50,	50,	50,	50,	50,	50, 50,	
    40,	40,	40,	40,	40,	40,	40,	40,
};

int8_t whiteKnightOpeningWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	5,	10,	10,	10,	10,	5,	0,
    5,	10,	40,	40,	40,	40,	10,	5,
    5,	10,	40,	60,	60,	40,	10,	5,
    5,	10,	40,	60,	60,	40,	10,	5,	
    5,	10,	40,	40,	40,	40,	10,	5,	
    0,	5,	10,	20,	20,	10,	5,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
};

int8_t whiteBishopOpeningWeights[64] = {
    0,	60,	40,	0,	0,	40,	60,	0,	
    10,	60,	40,	20,	20,	40,	60,	10,	
    0,	50,	50,	20,	20,	50,	50,	0,	
    0,	0,	40,	30,	30,	40,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,
};

int8_t whiteKingOpeningWeights[64] = {
    65,	60,	55,	50,	50,	55,	60,	65,
    10,	10,	10,	20,	20,	10,	10,	10,	
    0,	0,	0,	10,	10,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,
};

int8_t whiteQueenOpeningWeights[64] = {
    0,	0,	0,	0,	0,	0,	0,	0,
    0,	10,	10,	10,	10,	10,	10,	0,	
    0,	10,	40,	40,	40,	40,	10,	0,
    0,	10,	40,	40,	40,	40,	10,	0,
    0,	10,	40,	40,	40,	40,	10,	0,	
    0,	10,	40,	40,	40,	40,	10,	0,	
    0,	10,	10,	10,	10,	10,	10,	0,	
    0,	0,	0,	0,	0,	0,	0,	0,
};

int8_t whiteKingEndgameWeights[64] = {
    0,   10, 20, 30, 30, 20, 10,  0,
    10,  50, 50, 50, 50, 50, 50, 10,
    20,  50, 100,100,100,100,50, 20,
    30,  50, 100,120,120,100,50, 30,
    30,  50, 100,120,120,100,50, 30,
    20,  50, 100,100,100,100,50, 20,
    10,  50, 50, 50, 50, 50, 50, 10,
    0,   10, 20, 30, 30, 20, 10,  0,
};

int8_t blackPawnOpeningWeights[64];
int8_t blackRookOpeningWeights[64];
int8_t blackKnightOpeningWeights[64];
int8_t blackBishopOpeningWeights[64];
int8_t blackKingOpeningWeights[64];
int8_t blackQueenOpeningWeights[64];
int8_t blackKingEndgameWeights[64];

// "Rotates" the positional values to reflect the oposite colors positional value
// NB: King and Queen squares are interchanged
void m_inversePositionalValues(int8_t src[], int8_t dest[])
{
    for (uint8_t square = 0; square < 64; square++)
    {
        dest[square] = -src[63 - square];
    }
}

// Values of each of the pieces, indexed by the piece color and type
int64_t const pieceValues[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Black pieces
    -PAWN_VALUE,
    -ROOK_VALUE,
    -KNIGHT_VALUE,
    -BISHOP_VALUE,
    -KING_VALUE,
    -QUEEN_VALUE,
    0, 0,
    // White pieces
    PAWN_VALUE,
    ROOK_VALUE,
    KNIGHT_VALUE,
    BISHOP_VALUE,
    KING_VALUE,
    QUEEN_VALUE,
};

int8_t *openingPostitionalValues[] = {
    emptyWeights, 0, 0, 0, 0, 0, 0, 0, 0,
    // Black pieces
    blackPawnOpeningWeights,    // Pawn
    blackRookOpeningWeights,    // Rook
    blackKnightOpeningWeights,  // Knight
    blackBishopOpeningWeights,  // Bishop
    blackKingOpeningWeights,  // King
    blackQueenOpeningWeights,   // Queen
    0, 0,
    // White pieces
    whitePawnOpeningWeights,     // Pawn
    whiteRookOpeningWeights,     // Rook
    whiteKnightOpeningWeights,   // Knight
    whiteBishopOpeningWeights,   // Bishop
    whiteKingOpeningWeights,     // King
    whiteQueenOpeningWeights,    // Queen
};

int8_t *endgamePostitionalValues[] = {
    emptyWeights, 0, 0, 0, 0, 0, 0, 0, 0,
    // Black pieces
    blackPawnOpeningWeights,    // Pawn
    blackRookOpeningWeights,    // Rook
    blackKnightOpeningWeights,  // Knight
    blackBishopOpeningWeights,  // Bishop
    blackKnightOpeningWeights,  // King
    blackQueenOpeningWeights,   // Queen
    0, 0,
    // White pieces
    whitePawnOpeningWeights,     // Pawn
    whiteRookOpeningWeights,     // Rook
    whiteKnightOpeningWeights,   // Knight
    whiteBishopOpeningWeights,   // Bishop
    whiteKingEndgameWeights,     // King
    whiteQueenOpeningWeights,    // Queen
};

// Initialize the positional weights of the black player based on the tables of the 
void initEvalTables()
{
    m_inversePositionalValues(whitePawnOpeningWeights, blackPawnOpeningWeights);
    m_inversePositionalValues(whiteRookOpeningWeights, blackRookOpeningWeights);
    m_inversePositionalValues(whiteKnightOpeningWeights, blackKnightOpeningWeights);
    m_inversePositionalValues(whiteBishopOpeningWeights, blackBishopOpeningWeights);
    m_inversePositionalValues(whiteKingOpeningWeights, blackKingOpeningWeights);
    m_inversePositionalValues(whiteQueenOpeningWeights, blackQueenOpeningWeights);
    m_inversePositionalValues(whiteKingEndgameWeights, blackKingEndgameWeights);
}

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

    evaluation_t materialCount = 0;
    evaluation_t openingPositionalValue = 0;
    evaluation_t endgamePostitionalValue = 0;
    for (uint8_t square = 0; square < 64; square++)
    {
        materialCount += pieceValues[p_board->board[square]];
        openingPositionalValue += openingPostitionalValues[p_board->board[square]][square];
        endgamePostitionalValue += endgamePostitionalValues[p_board->board[square]][square];
    }

    evaluation_t openingEval = materialCount + openingPositionalValue;
    evaluation_t endgameEval = materialCount + endgamePostitionalValue;

    uint8_t phase = getPhase(p_board);
    evaluation_t eval = phase * endgameEval + (TOTAL_PHASE - phase) * openingEval;

    if (color == BLACK)
    {
        return -eval;
    }

    return eval;
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