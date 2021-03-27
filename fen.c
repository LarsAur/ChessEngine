#include <stdio.h>
#include "fen.h"
#include "hashing.h"

void createBoardFormFEN(char *FEN, Board *p_board)
{
    // Fill the board with empty squares before placing the piaces
    for (int8_t square = 0; square < 64; square++)
    {
        p_board->board[square] = EMPTY;
    }

    // Read the board position
    for (int8_t rank = 7; rank >= 0; rank--)
    {
        int8_t file = 0;
        while (*FEN != '/' && *FEN != ' ')
        {
            // Check for numbers (empty squares)
            if(*FEN <= '9' && *FEN >= '0')
            {
                // Move to the next file and move to the next FEN char
                file += *FEN - '0';
                FEN++;
                continue;
            }

            // Check the next piece
            uint8_t piece = EMPTY;
            switch (*FEN)
            {
                case 'q':
                case 'Q':
                    piece = QUEEN;
                    break;
                case 'k':
                    p_board->blackKing = file + rank * 8;
                    piece = KING;
                    break;
                case 'K':
                    p_board->whiteKing = file + rank * 8;
                    piece = KING;
                    break;
                case 'r':
                case 'R':
                    piece = ROOK;
                    break;
                case 'n':
                case 'N':
                    piece = KNIGHT;
                    break;
                case 'b':
                case 'B':
                    piece = BISHOP;
                    break;
                case 'p':
                case 'P':
                    piece = PAWN;
                    break;
                default:
                    printf("Illegal piece type char in FEN %c\n", *FEN);
                    break;
            }

            // Set the color of the piece based on the capitalization of the char
            // Non-capital letters => black
            if(*FEN >= 'a' && *FEN <= 'z')
            {
                piece |= BLACK;
            }
            // Capital letters => white
            else if(*FEN >= 'A' && *FEN <= 'Z')
            {
                piece |= WHITE;
            }
            //  Illegal character
            else
            {
                printf("Illegal color type of the char in FEN %c\n", *FEN);
            }

            // Put the piece on the bord and move to the next rank and FEN char
            p_board->board[file + rank*8] = piece;
            FEN++;
            file++;
        }
        FEN++;
    }

    // Read the game status
    // Set the turn
    if(*FEN == 'w'){
        p_board->turn = WHITE;
    }
    else if(*FEN == 'b')
    {
        p_board->turn = BLACK;
    }
    
    FEN += 2;
    // Castle Rights
    p_board->castleRights = 0;
    while (*FEN != ' ')
    {
        switch (*FEN)
        {
        case 'Q':
            p_board->castleRights |= WHITE_QUEEN_CASTLE_MASK;
            break;
        case 'q':
            p_board->castleRights |= BLACK_QUEEN_CASTLE_MASK;
            break;
        case 'K':
            p_board->castleRights |= WHITE_KING_CASTLE_MASK;
            break; 
        case 'k':
            p_board->castleRights |= BLACK_KING_CASTLE_MASK;
            break;
        case '-':
            p_board->castleRights = 0;
            break;
        }
        FEN++;
    }
    
    FEN++;
    // En passant target square in algebraic notation
    // After this section, the FEN pointer points towards the letter before the space
    if(*FEN == '-')
    {
        p_board->enPassantTarget = -1;
    }
    else
    {
        // Calculated as file + rank * 8 
        p_board->enPassantTarget = (*FEN - 'a') + (*(FEN++) - '0') * 8;
    }

    FEN+=2;
    p_board->halfMoves = *FEN - '0';

    FEN+=2;
    p_board->fullMoves = *FEN - '0';

    p_board->hash = zobristHash(p_board);
}