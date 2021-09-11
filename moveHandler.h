#ifndef MOVE_HANDLER_H

#define MOVE_HANDLER_H

#include <stdint.h>
#include "main.h"

typedef struct move_t
{
    uint8_t from;
    uint8_t to;

    uint8_t capture;        // The value of the piece that is captured, EMPTY is none is captured
    uint8_t castle;         // In the case of casteling, the king is to, from, and castle is set to 1
    uint8_t promotion;      // The piece to promote to
    uint8_t enPassant;      // Is the move en pessant
    uint8_t pawnDoubleMove; // Is the move a double move by a pawn, used to set enpassant target square

    // Information about the previous board
    uint8_t prevCastleRights;
    int8_t prevEnPassantTarget;
    uint64_t prevHash;
    uint8_t prevHalfMoves;
} Move;

typedef struct arraylist_t
{
    uint16_t size;      // Size of the current allocated array
    uint16_t elements;  // Number of elements in the allocated array
    Move* array;        // Pointer to the array of moves
} ArrayList;

ArrayList *getPseudoLegalMoves(Board *p_board);
ArrayList* getLegalMoves(Board *p_board);
void freeMoveList(ArrayList* p_list);
void performMove(Move *p_move, Board *p_board);
void undoMove(Move *p_move, Board *p_board);
void filterNonCaptureMoves(ArrayList* p_legalMovesList);
uint8_t isChecked(Board *p_board, uint8_t color);
int8_t isCheckmate(Board *p_board);

#endif // MOVE_HANDLER_H