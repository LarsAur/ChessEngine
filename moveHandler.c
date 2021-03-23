#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "moveHandler.h"
#include "utils.h"

void m_addPawnMoves(List *p_list, Board *p_board, uint8_t square);
void m_addRookMoves(List *p_list, Board *p_board, uint8_t square);
void m_addBishopMoves(List *p_list, Board *p_board, uint8_t square);
void m_addQueenMoves(List *p_list, Board *p_board, uint8_t square);
void m_addKingMoves(List *p_list, Board *p_board, uint8_t square);
void m_addKnightMoves(List *p_list, Board *p_board, uint8_t square);
void m_addDirectionalSlideMoves(List *p_list, Board *p_board, uint8_t square, int8_t dFile, int8_t dRank);
void m_removeNodeFromList(List *p_list, Node *p_node);

void m_filterCheckedMoves(List *p_list, Board *p_board);

void m_addMoveToList(List *p_list, uint8_t from, uint8_t to, uint8_t capture);
void m_addSpecialMoveToList(List *p_list, uint8_t from, uint8_t to, uint8_t capture, uint8_t castle, uint8_t promotion, uint8_t enpassant, uint8_t pawnDoubleMove);
void m_addMovePtrToList(List *p_list, Move *p_move);

uint8_t m_oppositeColor(uint8_t color);

List *getLegalMoves(Board *p_board)
{
    List *p_list = getPseudoLegalMoves(p_board);
    m_filterCheckedMoves(p_list, p_board);
    return p_list;
}

List *getPseudoLegalMoves(Board *p_board)
{
    List *p_list = malloc(sizeof(List));
    p_list->length = 0;
    p_list->p_tail = NULL;
    p_list->p_head = NULL;

    for (uint8_t square = 0; square < 64; square++)
    {
        // Go to next square if the color of the piece does not match the current players turn
        // Empty squares are also cought by this
        if ((p_board->board[square] & COLOR_MASK) != p_board->turn)
        {
            continue;
        }
        switch (p_board->board[square] & TYPE_MASK)
        {
        // Select the type of the non empty square, and add the possible moves
        case PAWN:
            m_addPawnMoves(p_list, p_board, square);
            break;
        case ROOK:
            m_addRookMoves(p_list, p_board, square);
            break;
        case KNIGHT:
            m_addKnightMoves(p_list, p_board, square);
            break;
        case BISHOP:
            m_addBishopMoves(p_list, p_board, square);
            break;
        case KING:
            m_addKingMoves(p_list, p_board, square);
            break;
        case QUEEN:
            m_addQueenMoves(p_list, p_board, square);
            break;
        }
    }

    return p_list;
}

// Adds the moves which the pawn on the square can perform to the list
void m_addPawnMoves(List *p_list, Board *p_board, uint8_t square)
{
    int8_t moveDirection = p_board->turn == WHITE ? 1 : -1;
    uint8_t initialRank = p_board->turn == WHITE ? 1 : 6;
    uint8_t promotionRank = p_board->turn == WHITE ? 7 : 0;

    uint8_t currentRank = square / 8;
    uint8_t currentFile = square % 8;

    // If pawns are placed on the promotion rank, they cannot move
    if (currentRank == promotionRank)
    {
        return;
    }

    uint8_t squareAhead = square + 8 * moveDirection;

    // Pawn Forward movement
    if (p_board->board[squareAhead] == EMPTY)
    {
        // Promotion
        if (currentRank == promotionRank - 1 * moveDirection)
        {
            m_addSpecialMoveToList(p_list, square, squareAhead, EMPTY, 0, QUEEN, 0, 0);
            m_addSpecialMoveToList(p_list, square, squareAhead, EMPTY, 0, ROOK, 0, 0);
            m_addSpecialMoveToList(p_list, square, squareAhead, EMPTY, 0, BISHOP, 0, 0);
            m_addSpecialMoveToList(p_list, square, squareAhead, EMPTY, 0, KNIGHT, 0, 0);
        }
        // Normal forward movement
        else
        {
            m_addMoveToList(p_list, square, squareAhead, EMPTY);
        }

        // Double forward move from initial rank
        if (currentRank == initialRank && p_board->board[squareAhead + 8 * moveDirection] == EMPTY)
        {
            m_addSpecialMoveToList(p_list, square, squareAhead + 8 * moveDirection, EMPTY, 0, 0, 0, 1);
        }
    }

    // Pawn attacks
    // Can take to the right (From white perspective)
    if (currentFile < 7)
    {
        uint8_t rightTarget = p_board->turn == WHITE ? square + 9 : square - 7;
        uint8_t capture = p_board->board[rightTarget];
        // Is the piece of the opposite color
        if ((p_board->board[rightTarget] & COLOR_MASK) == m_oppositeColor(p_board->turn))
        {
            // Captures into promotion
            if (currentRank == promotionRank - 1 * moveDirection)
            {
                m_addSpecialMoveToList(p_list, square, rightTarget, capture, 0, QUEEN, 0, 0);
                m_addSpecialMoveToList(p_list, square, rightTarget, capture, 0, ROOK, 0, 0);
                m_addSpecialMoveToList(p_list, square, rightTarget, capture, 0, BISHOP, 0, 0);
                m_addSpecialMoveToList(p_list, square, rightTarget, capture, 0, KNIGHT, 0, 0);
            }
            // Normal captures
            else
            {
                m_addMoveToList(p_list, square, rightTarget, capture);
            }
        }

        // Enpassant (Cannot lead to promotion)
        if (rightTarget == p_board->enPassantTarget)
        {
            // It is important that the capture in en passant is not empty even though the move-to is not the place of the capture
            // this is because it will be usefull when filtering for captures
            m_addSpecialMoveToList(p_list, square, rightTarget, p_board->board[p_board->enPassantTarget], 0, 0, 1, 0);
        }
    }

    // Can take to the left (from white perspective)
    if (currentFile > 0)
    {
        uint8_t leftTarget = p_board->turn == WHITE ? square + 7 : square - 9;
        uint8_t capture = p_board->board[leftTarget];
        // Is the piece of the opposite color
        if ((p_board->board[leftTarget] & COLOR_MASK) == m_oppositeColor(p_board->turn))
        {
            // Captures into promotion
            if (currentRank == promotionRank - 1 * moveDirection)
            {
                m_addSpecialMoveToList(p_list, square, leftTarget, capture, 0, QUEEN, 0, 0);
                m_addSpecialMoveToList(p_list, square, leftTarget, capture, 0, ROOK, 0, 0);
                m_addSpecialMoveToList(p_list, square, leftTarget, capture, 0, BISHOP, 0, 0);
                m_addSpecialMoveToList(p_list, square, leftTarget, capture, 0, KNIGHT, 0, 0);
            }
            // Normal captures
            else
            {
                m_addMoveToList(p_list, square, leftTarget, capture);
            }
        }

        // Enpassant (Cannot lead to promotion)
        if (leftTarget == p_board->enPassantTarget)
        {
            m_addSpecialMoveToList(p_list, square, leftTarget, p_board->board[p_board->enPassantTarget], 0, 0, 1, 0);
        }
    }
}

// Add rook moves
void m_addRookMoves(List *p_list, Board *p_board, uint8_t square)
{
    m_addDirectionalSlideMoves(p_list, p_board, square, 1, 0);
    m_addDirectionalSlideMoves(p_list, p_board, square, 0, 1);
    m_addDirectionalSlideMoves(p_list, p_board, square, -1, 0);
    m_addDirectionalSlideMoves(p_list, p_board, square, 0, -1);
}

// Add bishop moves
void m_addBishopMoves(List *p_list, Board *p_board, uint8_t square)
{
    m_addDirectionalSlideMoves(p_list, p_board, square, 1, 1);
    m_addDirectionalSlideMoves(p_list, p_board, square, 1, -1);
    m_addDirectionalSlideMoves(p_list, p_board, square, -1, 1);
    m_addDirectionalSlideMoves(p_list, p_board, square, -1, -1);
}

// Add all queen moves by combining the moves of a bishop and rook
void m_addQueenMoves(List *p_list, Board *p_board, uint8_t square)
{
    m_addRookMoves(p_list, p_board, square);
    m_addBishopMoves(p_list, p_board, square);
}

void m_addKingMoves(List *p_list, Board *p_board, uint8_t square)
{
    // Basic movement
    uint8_t currentRank = (square / 8);
    uint8_t currentFile = (square % 8);

    // Pairs of deltas
    int8_t dRank[8] = {0, 0, -1, 1, 1, 1, -1, -1};
    int8_t dFile[8] = {-1, 1, 0, 0, 1, -1, 1, -1};

    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t targetRank = currentRank + dRank[i];
        uint8_t targetFile = currentFile + dFile[i];

        // Check if the target is outside the board
        if (targetRank < 0 || targetRank > 7 || targetFile < 0 || targetFile > 7)
        {
            continue;
        }

        uint8_t targetSquare = targetFile + targetRank * 8;
        // Only move if the square is empty or is available for capture.
        // This will also catch the empty squares
        if ((p_board->board[targetSquare] & COLOR_MASK) != p_board->turn)
        {
            m_addMoveToList(p_list, square, targetSquare, p_board->board[targetSquare]);
        }
    }

    // Castling
    // This does not check for checks in between the king moves, and is done during the filter phase
    // This only checks if castle is available and if the squares are empty
    // The square of the king does not need to be checked, as it is involved in
    // the castleRights variables of the board
    uint8_t canCastleKing = p_board->castleRights & (p_board->turn == WHITE ? WHITE_KING_CASTLE_MASK : BLACK_KING_CASTLE_MASK);
    uint8_t canCastleQueen = p_board->castleRights & (p_board->turn == WHITE ? WHITE_QUEEN_CASTLE_MASK : BLACK_QUEEN_CASTLE_MASK);

    if (canCastleKing)
    {
        // Is the two squares to the right empty, NOTE: castling cannot capture
        if (p_board->board[square + 1] == EMPTY && p_board->board[square + 2] == EMPTY)
        {
            m_addSpecialMoveToList(p_list, square, square + 2, EMPTY, 1, 0, 0, 0);
        }
    }

    if (canCastleQueen)
    {
        // Is the three squares to the left empty, NOTE: castling cannot capture
        if (p_board->board[square - 1] == EMPTY && p_board->board[square - 2] == EMPTY && p_board->board[square - 3] == EMPTY)
        {
            m_addSpecialMoveToList(p_list, square, square - 2, EMPTY, 1, 0, 0, 0);
        }
    }
}

void m_addKnightMoves(List *p_list, Board *p_board, uint8_t square)
{
    uint8_t currentRank = (square / 8);
    uint8_t currentFile = (square % 8);

    // Pairs of deltas
    int8_t dRank[8] = {1, 2, 2, 1, -1, -2, -2, -1};
    int8_t dFile[8] = {-2, -1, 1, 2, 2, 1, -1, -2};

    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t targetRank = currentRank + dRank[i];
        uint8_t targetFile = currentFile + dFile[i];

        // Check if the target is outside the board
        if (targetRank < 0 || targetRank > 7 || targetFile < 0 || targetFile > 7)
        {
            continue;
        }

        uint8_t targetSquare = targetFile + targetRank * 8;
        // Only move if the square is empty or is available for capture.
        // This will also catch the empty squares
        if ((p_board->board[targetSquare] & COLOR_MASK) != p_board->turn)
        {
            m_addMoveToList(p_list, square, targetSquare, p_board->board[targetSquare]);
        }
    }
}

// Adds all move in a direction until it hits a piece, if the piece is of the opposite color that move is included
void m_addDirectionalSlideMoves(List *p_list, Board *p_board, uint8_t square, int8_t dFile, int8_t dRank)
{
    uint8_t currentRank = (square / 8);
    uint8_t currentFile = (square % 8);

    for (int8_t i = 1; i < 8; i++)
    {
        int8_t targetRank = currentRank + i * dRank; // CurrentRank + RankDiff
        int8_t targetFile = currentFile + i * dFile; // CurrentRank + FileDiff

        // Check if the target is outside the board
        if (targetRank < 0 || targetRank > 7 || targetFile < 0 || targetFile > 7)
        {
            return;
        }

        uint8_t targetSquare = targetFile + targetRank * 8;
        if (p_board->board[targetSquare] == EMPTY)
        {
            m_addMoveToList(p_list, square, targetSquare, EMPTY);
        }
        else
        {
            // If the square contains a piece of the opposite color, take it and return. Else do not take the piece and return.
            if ((p_board->board[targetSquare] & COLOR_MASK) == m_oppositeColor(p_board->turn))
            {
                m_addMoveToList(p_list, square, targetSquare, p_board->board[targetSquare]);
            }
            return;
        }
    }
}

void m_filterCheckedMoves(List *p_list, Board *p_board)
{
    Board tmpBoard;

    Node *p_node = p_list->p_head;

    while (p_node != NULL)
    {
        memcpy(&tmpBoard, p_board, sizeof(Board));
        performMove(p_node->p_move , &tmpBoard);

        // Find the square with the king
        // The color is the color of the turn of p_board but the position is from the new board
        uint8_t kingSquare = p_board->turn == WHITE ? tmpBoard.whiteKing : tmpBoard.blackKing;

        Node *tmp = p_node->p_next;

        if (isChecked(&tmpBoard, kingSquare))
        {
            m_removeNodeFromList(p_list, p_node);
        }
        // Check if the in-between square is targetet by the opponent and the king square
        else if (p_node->p_move->castle)
        {
            memcpy(&tmpBoard, p_board, sizeof(Board));

            // Check if king is in check before casteling (not after as above)
            tmpBoard.turn = m_oppositeColor(tmpBoard.turn); // Switching turn is needed for isChecked to find the correct moves
            if (isChecked(&tmpBoard, (p_board->turn == WHITE ? 4 : 60)))
            {
                m_removeNodeFromList(p_list, p_node);
            }
            else
            {
                // Switching the turn back
                tmpBoard.turn = m_oppositeColor(tmpBoard.turn);
                // Check if king is checked in the in-between squares
                Move middleMove;
                middleMove = *(p_node->p_move);
                middleMove.castle = 0;
                // Castle is kingside
                if (p_node->p_move->to > p_node->p_move->from)
                {
                    middleMove.to = middleMove.to - 1;
                }
                else
                // Castle is queenside
                {
                    middleMove.to = middleMove.to + 1;
                }

                performMove(&middleMove, &tmpBoard);

                if (isChecked(&tmpBoard, middleMove.to))
                {
                    m_removeNodeFromList(p_list, p_node);
                }
            }
        }

        p_node = tmp;
    }
}

// Removes and frees all moves which are not captures from the list
void filterNonCaptureMoves(List* p_legalMovesList)
{
    Node *p_node = p_legalMovesList->p_head;
    while (p_node != NULL)
    {
        Node *tmp = p_node->p_next;
        if(p_node->p_move->capture == EMPTY)
        {
            m_removeNodeFromList(p_legalMovesList, p_node);
        }

        p_node = tmp;
    }
    
}

// Returns wether the king is in check in a board state
uint8_t isChecked(Board *p_board, uint8_t kingSquare)
{
    List *p_pseudoLegalMoves = getPseudoLegalMoves(p_board);
    Node *p_node = p_pseudoLegalMoves->p_head;
    while (p_node != NULL)
    {
        if (p_node->p_move->to == kingSquare)
        {
            freeMoveList(p_pseudoLegalMoves);
            return 1;
        }

        p_node = p_node->p_next;
    }

    freeMoveList(p_pseudoLegalMoves);
    return 0;
}

// This also adds some information to the move about the availability of casteling before the move was made
// and the potential previous en passant target.
void performMove(Move *p_move, Board *p_board)
{
    // Add information about the current position before making the move, such that moves can be undone
    p_move->prevCastleRights = p_board->castleRights;
    p_move->prevEnPassantTarget = p_board->enPassantTarget;

    // Perform the basic move
    p_board->board[p_move->to] = p_board->board[p_move->from];
    p_board->board[p_move->from] = EMPTY;

    if (p_move->from == p_board->whiteKing)
    {
        p_board->whiteKing = p_move->to;
    }
    else if (p_move->from == p_board->blackKing)
    {
        p_board->blackKing = p_move->to;
    }

    // Move the rook when castling
    if (p_move->castle)
    {
        // Castle king side
        if (p_move->to > p_move->from)
        {
            // Place the rook to the left of the king, castling is only available if the king has not moved
            p_board->board[p_move->to - 1] = p_board->board[p_move->to + 1];
            p_board->board[p_move->to + 1] = EMPTY;
        }
        else
        // Castle queen side
        {
            // Place the rook to the right of the king, castling is only available if the king has not moved
            p_board->board[p_move->to + 1] = p_board->board[p_move->to - 2];
            p_board->board[p_move->to - 2] = EMPTY;
        }
    }

    // Remove the double move pawn on en-passant
    if (p_move->enPassant)
    {
        p_board->board[p_board->turn == WHITE ? p_move->to - 8 : p_move->to + 8] = EMPTY;
    }

    // Set the en passant target when a pawn moves two squares, the target is removed if it is not a double move
    if (p_move->pawnDoubleMove)
    {
        p_board->enPassantTarget = p_board->turn == WHITE ? p_move->to - 8 : p_move->to + 8;
    }
    else
    {
        p_board->enPassantTarget = -1;
    }

    // If a pawn promotes, put the promoted piece on the 'to' square
    if (p_move->promotion)
    {
        p_board->board[p_move->to] = p_move->promotion | (p_board->board[p_move->to] & COLOR_MASK);
    }

    // Remove potential castle rights if the king moves
    if (p_move->from == 4)
    {
        p_board->castleRights &= (~(WHITE_KING_CASTLE_MASK | WHITE_QUEEN_CASTLE_MASK) & 0xf); // Set the KING and QUEEN castle rights to 0 for white while keeping blacks
    }
    else if (p_move->from == 60)
    {
        p_board->castleRights &= (~(BLACK_KING_CASTLE_MASK | BLACK_QUEEN_CASTLE_MASK) & 0xf); // Set the KING and QUEEN castle rights to 0 for black while keeping whites
    }

    // Remove potential castle rights if the rooks move or are taken
    // This cannot be if else, because of the case where a1 rook takes a8 rook
    if (p_move->from == 0 || p_move->to == 0)
    {
        p_board->castleRights &= (~(WHITE_QUEEN_CASTLE_MASK) & 0xf);
    }

    if (p_move->from == 7 || p_move->to == 7)
    {
        p_board->castleRights &= (~(WHITE_KING_CASTLE_MASK) & 0xf);
    }

    if (p_move->from == 56 || p_move->to == 56)
    {
        p_board->castleRights &= (~(BLACK_QUEEN_CASTLE_MASK) & 0xf);
    }

    if (p_move->from == 63 || p_move->to == 63)
    {
        p_board->castleRights &= (~(BLACK_KING_CASTLE_MASK) & 0xf);
    }

    // Change turns
    p_board->turn = m_oppositeColor(p_board->turn);

    // Update the hash
}

// Perform the opposite actions in the opposite order to performMove()
void undoMove(Move *p_move, Board *p_board)
{
    // Change turn back
    p_board->turn = m_oppositeColor(p_board->turn);

    // Give back potential castle rights
    p_board->castleRights = p_move->prevCastleRights;

    // If a promotion was made turn the piece back to a pawn
    if (p_move->promotion)
    {
        p_board->board[p_move->to] = PAWN | (p_board->board[p_move->to] & COLOR_MASK);
    }

    // Set the potential previous en passant target
    p_board->enPassantTarget = p_move->prevEnPassantTarget;

    // In the case of en passant, place back the pawn
    if (p_move->enPassant)
    {
        p_board->board[p_board->turn == WHITE ? p_move->to - 8 : p_move->to + 8] = PAWN | m_oppositeColor(p_board->turn);
    }

    // Move the rook back if castling happened
    if (p_move->castle)
    {
        // Castle was king side
        if (p_move->to > p_move->from)
        {
            // Place the rook back to its original position, p_move->to is the kings position after the castle
            p_board->board[p_move->to + 1] = p_board->board[p_move->to - 1];
            p_board->board[p_move->to - 1] = EMPTY;
        }
        else
        // Castle was queen side
        {
            // Place the rook back to its original position, p_move->to is the kings position after the castle
            p_board->board[p_move->to - 2] = p_board->board[p_move->to + 1];
            p_board->board[p_move->to + 1] = EMPTY;
        }
    }

    // Maintain the information about the kings position
    if (p_move->to == p_board->whiteKing)
    {
        p_board->whiteKing = p_move->from;
    }
    else if (p_move->to == p_board->blackKing)
    {
        p_board->blackKing = p_move->from;
    }

    // Move the pieces back and replace potential captured pieces
    p_board->board[p_move->from] = p_board->board[p_move->to];
    if (!p_move->enPassant)
    {
        p_board->board[p_move->to] = p_move->capture;
    }
    else
    {
        // the enpassant pawn is placed earlier in undoMove()
        p_board->board[p_move->to] = EMPTY;
    }
}

// Free all elements of the list, including the list itself
void freeMoveList(List *p_list)
{
    Node *p_node = p_list->p_head;
    while (p_node != NULL)
    {
        Node *p_tmp = p_node;
        p_node = p_node->p_next;
        free(p_tmp->p_move);
        free(p_tmp);
    }
    free(p_list);
}

// Remove and free the node and move from a list
void m_removeNodeFromList(List *p_list, Node *p_node)
{
    p_list->length--;

    // Remove head
    if (p_node == p_list->p_head)
    {
        // If this is the only element in the list
        // checking against tail and not p_next in case of a link error
        if (p_node == p_list->p_tail)
        {
            p_list->p_head = NULL;
            p_list->p_tail = NULL;
        }
        else
        {
            p_list->p_head = p_node->p_next;
            p_node->p_next->p_prev = NULL;
        }
    }
    // Remove the last element
    else if (p_list->p_tail == p_node)
    {
        p_node->p_prev->p_next = NULL;
        p_list->p_tail = p_node->p_prev;
    }
    // Remove middle element
    else
    {
        p_node->p_prev->p_next = p_node->p_next;
        p_node->p_next->p_prev = p_node->p_prev;
    }

    free(p_node->p_move);
    free(p_node);
}

// Adds a move containing a castle, promotion, enpassant or pawn double move to the list
void m_addSpecialMoveToList(List *p_list, uint8_t from, uint8_t to, uint8_t capture, uint8_t castle, uint8_t promotion, uint8_t enpassant, uint8_t pawnDoubleMove)
{
    Move *p_move = malloc(sizeof(Move));
    p_move->from = from;
    p_move->to = to;
    p_move->capture = capture;
    p_move->castle = castle;
    p_move->promotion = promotion;
    p_move->enPassant = enpassant;
    p_move->pawnDoubleMove = pawnDoubleMove;
    m_addMovePtrToList(p_list, p_move);
}

// Adds a basic move (from, to) to the list
void m_addMoveToList(List *p_list, uint8_t from, uint8_t to, uint8_t capture)
{
    Move *p_move = malloc(sizeof(Move));
    p_move->from = from;
    p_move->to = to;
    p_move->capture = capture;
    p_move->castle = 0;
    p_move->promotion = 0;
    p_move->enPassant = 0;
    p_move->pawnDoubleMove = 0;
    m_addMovePtrToList(p_list, p_move);
}

// Takes the pointer to the move, creates a node and adds it to the list
void m_addMovePtrToList(List *p_list, Move *p_move)
{
    Node *p_node = malloc(sizeof(Node));
    p_node->p_move = p_move;
    p_node->p_next = NULL;
    p_node->p_prev = p_list->p_tail;

    if (p_list->p_head == NULL)
    {
        p_list->p_head = p_node;
    }

    if (p_list->p_tail != NULL)
    {
        p_list->p_tail->p_next = p_node;
    }

    p_list->p_tail = p_node;
    p_list->length++;
}

uint8_t m_oppositeColor(uint8_t color)
{
    if (color == WHITE)
    {
        return BLACK;
    }
    else if (color == BLACK)
    {
        return WHITE;
    }

    printf("Cannot find the opposite color of %d", color);
    return EMPTY;
}