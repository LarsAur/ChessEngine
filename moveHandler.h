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

} Move;

typedef struct node_t
{
    struct node_t *p_next;
    struct node_t *p_prev;
    struct move_t *p_move;
} Node;

typedef struct list_t
{
    uint16_t length;
    Node *p_head;
    Node *p_tail;
} List;

List *getPseudoLegalMoves(Board *p_board);
List* getLegalMoves(Board *p_board);
void freeMoveList(List* p_list);
void performMove(Move *p_move, Board *p_board);
void undoMove(Move *p_move, Board *p_board);
void filterNonCaptureMoves(List* p_legalMovesList);
uint8_t isChecked(Board *p_board, uint8_t color);

#endif // MOVE_HANDLER_H