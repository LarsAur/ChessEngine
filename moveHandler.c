#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "moveHandler.h"
#include "utils.h"
#include "hashing.h"

void m_addPawnMoves(ArrayList *p_list, Board *p_board, uint8_t square);
void m_addRookMoves(ArrayList *p_list, Board *p_board, uint8_t square);
void m_addBishopMoves(ArrayList *p_list, Board *p_board, uint8_t square);
void m_addQueenMoves(ArrayList *p_list, Board *p_board, uint8_t square);
void m_addKingMoves(ArrayList *p_list, Board *p_board, uint8_t square);
void m_addKnightMoves(ArrayList *p_list, Board *p_board, uint8_t square);
void m_addDirectionalSlideMoves(ArrayList *p_list, Board *p_board, uint8_t square, int8_t dFile, int8_t dRank);

void m_filterCheckedMoves(ArrayList *p_list, Board *p_board);
uint8_t m_isSlidingChecked(Board *p_board, uint8_t kingRank, uint8_t kingFile, int8_t dfile, int8_t drank, uint8_t type, uint8_t oppositeColor);

void m_addMoveToList(ArrayList *p_list, uint8_t from, uint8_t to, uint8_t capture);
void m_addSpecialMoveToList(ArrayList *p_list, uint8_t from, uint8_t to, uint8_t capture, uint8_t castle, uint8_t promotion, uint8_t enpassant, uint8_t pawnDoubleMove);

uint8_t m_oppositeColor(uint8_t color);

ArrayList *getLegalMoves(Board *p_board)
{
    ArrayList *p_list = getPseudoLegalMoves(p_board);
    m_filterCheckedMoves(p_list, p_board);
    return p_list;
}

ArrayList *getPseudoLegalMoves(Board *p_board)
{
    ArrayList *p_list = malloc(sizeof(ArrayList));
    p_list->size = 16;
    p_list->elements = 0;
    p_list->array = malloc(sizeof(Move) * p_list->size);

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
void m_addPawnMoves(ArrayList *p_list, Board *p_board, uint8_t square)
{
    int8_t moveDirection;
    uint8_t initialRank;
    uint8_t promotionRank;

    if (p_board->turn == WHITE)
    {
        moveDirection = 1;
        initialRank = 1;
        promotionRank = 7;
    }
    else
    {
        moveDirection = -1;
        initialRank = 6;
        promotionRank = 0;
    }

    uint8_t currentRank = square >> 3; // Devide by 8 (2^3)
    uint8_t currentFile = square % 8;

    // If pawns are placed on the promotion rank, they cannot move
    // This will never happen (It might happen in a test)
    /*if (currentRank == promotionRank)
    {
        return;
    }*/

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

            // Double forward move from initial rank
            if (currentRank == initialRank && p_board->board[squareAhead + 8 * moveDirection] == EMPTY)
            {
                m_addSpecialMoveToList(p_list, square, squareAhead + 8 * moveDirection, EMPTY, 0, 0, 0, 1);
            }
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
void m_addRookMoves(ArrayList *p_list, Board *p_board, uint8_t square)
{
    m_addDirectionalSlideMoves(p_list, p_board, square, 1, 0);
    m_addDirectionalSlideMoves(p_list, p_board, square, 0, 1);
    m_addDirectionalSlideMoves(p_list, p_board, square, -1, 0);
    m_addDirectionalSlideMoves(p_list, p_board, square, 0, -1);
}

// Add bishop moves
void m_addBishopMoves(ArrayList *p_list, Board *p_board, uint8_t square)
{
    m_addDirectionalSlideMoves(p_list, p_board, square, 1, 1);
    m_addDirectionalSlideMoves(p_list, p_board, square, 1, -1);
    m_addDirectionalSlideMoves(p_list, p_board, square, -1, 1);
    m_addDirectionalSlideMoves(p_list, p_board, square, -1, -1);
}

// Add all queen moves by combining the moves of a bishop and rook
void m_addQueenMoves(ArrayList *p_list, Board *p_board, uint8_t square)
{
    m_addRookMoves(p_list, p_board, square);
    m_addBishopMoves(p_list, p_board, square);
}

void m_addKingMoves(ArrayList *p_list, Board *p_board, uint8_t square)
{
    // Basic movement
    int8_t currentRank = (square / 8);
    int8_t currentFile = (square % 8);

    // Pairs of deltas
    int8_t dRank[8] = {0, 0, -1, 1, 1, 1, -1, -1};
    int8_t dFile[8] = {-1, 1, 0, 0, 1, -1, 1, -1};

    for (uint8_t i = 0; i < 8; i++)
    {
        int8_t targetRank = currentRank + dRank[i];
        int8_t targetFile = currentFile + dFile[i];

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

void m_addKnightMoves(ArrayList *p_list, Board *p_board, uint8_t square)
{
    uint8_t currentRank = (square / 8);
    uint8_t currentFile = (square % 8);

    // Pairs of deltas
    int8_t dRank[8] = {1, 2, 2, 1, -1, -2, -2, -1};
    int8_t dFile[8] = {-2, -1, 1, 2, 2, 1, -1, -2};

    for (uint8_t i = 0; i < 8; i++)
    {
        int8_t targetRank = currentRank + dRank[i];
        int8_t targetFile = currentFile + dFile[i];

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
void m_addDirectionalSlideMoves(ArrayList *p_list, Board *p_board, uint8_t square, int8_t dFile, int8_t dRank)
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

void m_filterCheckedMoves(ArrayList *p_list, Board *p_board)
{
    Board tmpBoard;

    uint16_t writeIndex = 0;
    for (uint16_t readIndex = 0; readIndex < p_list->elements; readIndex++)
    {
        memcpy(&tmpBoard, p_board, sizeof(Board));
        performMove(&p_list->array[readIndex], &tmpBoard);

        if (isChecked(&tmpBoard, p_board->turn))
        {
            // Remove the move from the list, by not including it
            continue;
        }
        // Check if the in-between square is targetet by the opponent and the king square
        else if (p_list->array[readIndex].castle)
        {
            // Check if king is in check before casteling (not after as above)
            if (isChecked(p_board, p_board->turn))
            {
                // Remove the move from the list, by not including it
                continue;
            }
            else
            {
                memcpy(&tmpBoard, p_board, sizeof(Board));
                // Switching the turn back TODO remove ????
                tmpBoard.turn = m_oppositeColor(tmpBoard.turn);

                // Check if king is checked in the in-between squares
                Move middleMove;
                middleMove = p_list->array[readIndex];
                middleMove.castle = 0;

                // Castle is kingside
                if (middleMove.to > middleMove.from)
                {
                    middleMove.to = middleMove.to - 1;
                }
                else
                // Castle is queenside
                {
                    middleMove.to = middleMove.to + 1;
                }

                performMove(&middleMove, &tmpBoard);

                if (isChecked(&tmpBoard, p_board->turn))
                {
                    // Remove the move from the list, by not including it
                    continue;
                }
            }
        }

        p_list->array[writeIndex] = p_list->array[readIndex];
        writeIndex++;
    }

    p_list->elements = writeIndex;
}

// Removes and frees all moves which are not captures from the list
void filterNonCaptureMoves(ArrayList *p_legalMovesList)
{
    uint16_t writeIndex = 0;
    for (uint16_t readIndex = 0; readIndex < p_legalMovesList->elements; readIndex++)
    {
        if (p_legalMovesList->array[readIndex].capture != EMPTY)
        {
            p_legalMovesList->array[writeIndex] = p_legalMovesList->array[readIndex];
            writeIndex++;
        }
    }

    // Set the new number of elements
    p_legalMovesList->elements = writeIndex;
}

// This is an "expensive" function checking if the current player has no moves and if it is checkmate/stalemate
// In the case of checkmate the color of the winner is returned. In the case of stalemate 1 is returned. 0 is returned if legal moves exist.
// The function also consider board repetition, and 50 move rule
int8_t isCheckmate(Board *p_board)
{
    // Stalemate on 50 move rule
    if (p_board->halfMoves == 50)
    {
        return 1;
    }

    // Stalemate board repetition
    uint16_t tailIndex = (p_board->fullMoves - 1) * 2 + (p_board->turn == BLACK);
    uint8_t repeats = 0;
    for (uint16_t i = 0; i < tailIndex; i++)
    {
        repeats += p_board->gameHashHistory[i] == p_board->hash;
    }

    // The board is only added to the list once a move is performed 'from' the board
    // Thus the position is repeated 3 times if it is seen 2 times before
    if (repeats >= 2)
    {
        return 1;
    }

    ArrayList *p_legalMoves = getLegalMoves(p_board);
    uint8_t num_moves = p_legalMoves->elements;
    freeMoveList(p_legalMoves);

    // Legal moves exist
    if (num_moves > 0)
    {
        return 0;
    }

    if (isChecked(p_board, p_board->turn))
    {
        return p_board->turn == WHITE ? BLACK : WHITE;
    }

    // If the current player is not in check it is stalemate
    return 1;
}

// Returns wether the king is in check in a board state
uint8_t isChecked(Board *p_board, uint8_t color)
{
    uint8_t oppositeColor = m_oppositeColor(color);
    uint8_t kingSquare = color == WHITE ? p_board->whiteKing : p_board->blackKing;

    // Check all possible psitions which can target the king square
    uint8_t kingRank = kingSquare >> 3;    // Devide by 8
    uint8_t kingFile = kingSquare & 0b111; // modulo 8

    // Pawns on diagonal
    // White pawns cannot attack something on rank 0 and 1
    if (color == BLACK && kingRank > 1)
    {
        // Down left
        if (kingFile > 0 && p_board->board[kingSquare - 9] == (PAWN | WHITE))
        {
            return 1;
        }

        // Down right
        if (kingFile < 7 && p_board->board[kingSquare - 7] == (PAWN | WHITE))
        {
            return 1;
        }
    }
    // Black pawn cannot attack something on rank 6 or 7
    else if (color == WHITE && kingRank < 6)
    {
        // Up left
        if (kingFile > 0 && (p_board->board[kingSquare + 7] == (PAWN | BLACK)))
        {
            return 1;
        }

        // Up right
        if (kingFile < 7 && (p_board->board[kingSquare + 9] == (PAWN | BLACK)))
        {
            return 1;
        }
    }

    // Knights in a knight move away

    // Pairs of deltas for knight moves
    int8_t knightDRank[8] = {1, 2, 2, 1, -1, -2, -2, -1};
    int8_t knightDFile[8] = {-2, -1, 1, 2, 2, 1, -1, -2};

    for (uint8_t i = 0; i < 8; i++)
    {
        int8_t targetRank = kingRank + knightDRank[i];
        int8_t targetFile = kingFile + knightDFile[i];

        // Check if the target is inside the board
        if (targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7)
        {
            int8_t targetSquare = targetFile + targetRank * 8;
            // Check if  the target square is a knight of the correct color
            if (p_board->board[targetSquare] == (KNIGHT | oppositeColor))
            {
                return 1;
            }
        }
    }

    // King in any immediate square

    // Pairs of deltas for king moves
    int8_t kingDRank[8] = {0, 0, -1, 1, 1, 1, -1, -1};
    int8_t kingDFile[8] = {-1, 1, 0, 0, 1, -1, 1, -1};

    for (uint8_t i = 0; i < 8; i++)
    {
        int8_t targetRank = kingRank + kingDRank[i];
        int8_t targetFile = kingFile + kingDFile[i];

        // Check if the target is inside the board
        if (targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7)
        {
            int8_t targetSquare = targetFile + targetRank * 8;
            // Check if  the target square is a knight of the correct color
            if (p_board->board[targetSquare] == (KING | oppositeColor))
            {
                return 1;
            }
        }
    }

    // Queen or bishop on diagonal
    if (m_isSlidingChecked(p_board, kingRank, kingFile, 1, 1, BISHOP, oppositeColor))
        return 1;
    if (m_isSlidingChecked(p_board, kingRank, kingFile, -1, 1, BISHOP, oppositeColor))
        return 1;
    if (m_isSlidingChecked(p_board, kingRank, kingFile, 1, -1, BISHOP, oppositeColor))
        return 1;
    if (m_isSlidingChecked(p_board, kingRank, kingFile, -1, -1, BISHOP, oppositeColor))
        return 1;

    // Queen or rook on file/rank
    if (m_isSlidingChecked(p_board, kingRank, kingFile, 0, 1, ROOK, oppositeColor))
        return 1;
    if (m_isSlidingChecked(p_board, kingRank, kingFile, 0, -1, ROOK, oppositeColor))
        return 1;
    if (m_isSlidingChecked(p_board, kingRank, kingFile, 1, 0, ROOK, oppositeColor))
        return 1;
    if (m_isSlidingChecked(p_board, kingRank, kingFile, -1, 0, ROOK, oppositeColor))
        return 1;

    return 0;
}

// Checks in the direction of dfile and drank for the piece type including queen
// type should be ROOK when one of the directions are 0 and the other is 1/-1 and BISHOP when the direction is diagonal
// The function returns whether there is a piece (which can target the king) in the direction
uint8_t m_isSlidingChecked(Board *p_board, uint8_t kingRank, uint8_t kingFile, int8_t dfile, int8_t drank, uint8_t type, uint8_t oppositeColor)
{
    uint8_t i = 1;
    // While the search square is inside the board
    while (kingRank + drank * i <= 7 && kingRank + drank * i >= 0 && kingFile + dfile * i <= 7 && kingFile + dfile * i >= 0)
    {
        uint8_t square = kingFile + dfile * i + (kingRank + drank * i) * 8;
        if (p_board->board[square] != EMPTY)
        {
            if (p_board->board[square] == (oppositeColor | type) || p_board->board[square] == (oppositeColor | QUEEN))
            {
                return 1;
            }

            // Something is blocking
            return 0;
        }
        i++;
    }

    return 0;
}

// This also adds some information to the move about the availability of casteling before the move was made
// and the potential previous en passant target.
void performMove(Move *p_move, Board *p_board)
{
    // Append the current possition to the board hostory
    p_board->gameHashHistory[(p_board->fullMoves - 1) * 2 + (p_board->turn == BLACK)] = p_board->hash;

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
        p_board->castleRights &= (~(WHITE_QUEEN_CASTLE_MASK)&0xf);
    }

    if (p_move->from == 7 || p_move->to == 7)
    {
        p_board->castleRights &= (~(WHITE_KING_CASTLE_MASK)&0xf);
    }

    if (p_move->from == 56 || p_move->to == 56)
    {
        p_board->castleRights &= (~(BLACK_QUEEN_CASTLE_MASK)&0xf);
    }

    if (p_move->from == 63 || p_move->to == 63)
    {
        p_board->castleRights &= (~(BLACK_KING_CASTLE_MASK)&0xf);
    }

    // Update the full move counter
    // Only incremented after black moves
    if (p_board->turn == BLACK)
    {
        p_board->fullMoves++;
    }

    // Update the number of half moves since last pawn advance or piece capture
    p_move->prevHalfMoves = p_board->halfMoves;
    if (p_move->capture)
    {
        p_board->halfMoves = 0;
    }
    else if ((p_board->board[p_move->to] & TYPE_MASK) == PAWN)
    {
        p_board->halfMoves = 0;
    }
    else
    {
        p_board->halfMoves++;
    }

    // Change turns
    p_board->turn = m_oppositeColor(p_board->turn);

    // Update the hash of the new board, and save the previous hash
    p_move->prevHash = p_board->hash;
    updateZobristHash(p_board, p_move);
}

// Perform the opposite actions in the opposite order to performMove()
void undoMove(Move *p_move, Board *p_board)
{
    // If black just performed a move, decrease the fullmove counter
    if (p_board->turn == WHITE)
    {
        p_board->fullMoves--;
    }

    // Set the half moves back to the value before the move
    p_board->halfMoves = p_move->prevHalfMoves;

    // Set the hash back to the value before the move
    p_board->hash = p_move->prevHash;

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

    // Remove the position from the board history
    p_board->gameHashHistory[(p_board->fullMoves - 1) * 2 + (p_board->turn == BLACK)] = 0;
}

// Free all elements of the list, including the list itself
void freeMoveList(ArrayList *p_list)
{
    free(p_list->array);
    free(p_list);
}

// Adds a move containing a castle, promotion, enpassant or pawn double move to the list
void m_addSpecialMoveToList(ArrayList *p_list, uint8_t from, uint8_t to, uint8_t capture, uint8_t castle, uint8_t promotion, uint8_t enpassant, uint8_t pawnDoubleMove)
{
    // Double the size of the array list if there is no more space
    if (p_list->elements == p_list->size)
    {
        p_list->array = realloc(p_list->array, p_list->size * 2 * sizeof(Move));
        p_list->size *= 2;
    }

    // Set the paramaters of the move in the array list
    p_list->array[p_list->elements].from = from;
    p_list->array[p_list->elements].to = to;
    p_list->array[p_list->elements].capture = capture;
    p_list->array[p_list->elements].castle = castle;
    p_list->array[p_list->elements].promotion = promotion;
    p_list->array[p_list->elements].enPassant = enpassant;
    p_list->array[p_list->elements].pawnDoubleMove = pawnDoubleMove;
    p_list->elements++;
}

// Adds a basic move (from, to) to the list
void m_addMoveToList(ArrayList *p_list, uint8_t from, uint8_t to, uint8_t capture)
{
    // Double the size of the array list if there is no more space
    if (p_list->elements == p_list->size)
    {
        p_list->array = realloc(p_list->array, p_list->size * 2 * sizeof(Move));
        p_list->size *= 2;
    }

    // Set the paramaters of the move in the array list
    p_list->array[p_list->elements].from = from;
    p_list->array[p_list->elements].to = to;
    p_list->array[p_list->elements].capture = capture;
    p_list->array[p_list->elements].castle = 0;
    p_list->array[p_list->elements].promotion = 0;
    p_list->array[p_list->elements].enPassant = 0;
    p_list->array[p_list->elements].pawnDoubleMove = 0;
    p_list->elements++;
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