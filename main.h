#ifndef MAIN_H

#define MAIN_H

#include <stdint.h>
#include "hashing.h"

#define COLOR_MASK 0b11000
#define BLACK   0b01000
#define WHITE   0b10000

#define TYPE_MASK 0b111
#define PAWN    0b001
#define ROOK    0b010
#define KNIGHT  0b011
#define BISHOP  0b100
#define KING    0b101
#define QUEEN   0b110

#define WHITE_KING_CASTLE_MASK  0b0001
#define WHITE_QUEEN_CASTLE_MASK 0b0010
#define BLACK_KING_CASTLE_MASK  0b0100
#define BLACK_QUEEN_CASTLE_MASK 0b1000

#define EMPTY 0b00000

typedef struct board_t
{
    uint8_t board[64];

    uint8_t castleRights;
    uint8_t turn;       // Black or whites turn to move
    uint8_t whiteKing;
    uint8_t blackKing;

    uint16_t fullMoves; // Increments after black has moved
    uint16_t halfMoves; // Number of moves since the last capture or the last pawn move

    int8_t enPassantTarget;

    uint64_t hash;
} Board;

#endif // MAIN_H