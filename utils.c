#include <stdio.h>
#include "main.h"
#include "utils.h"
#include "moveHandler.h"

void printBoard(Board *p_board)
{
    printf("__|Turn: %s|__\n", p_board->turn == WHITE ? "White" : "Black");
    for (int8_t rank = 7; rank >= 0; rank--)
    {
        char line[18];
        char *square = line;
        for (int8_t file = 0; file < 8; file++)
        {
            uint8_t piece = p_board->board[file + rank * 8];

            if (piece == EMPTY)
            {
                // Create black and white empty squares
                if ((file + rank) % 2 == 0)
                {
                    *square = ' ';
                }
                else
                {
                    *square = ' ';
                }
            }
            else
            {
                // Select the type of the non empty square
                switch (piece & TYPE_MASK)
                {
                case PAWN:
                    *square = 'p';
                    break;
                case ROOK:
                    *square = 'r';
                    break;
                case KNIGHT:
                    *square = 'n';
                    break;
                case BISHOP:
                    *square = 'b';
                    break;
                case KING:
                    *square = 'k';
                    break;
                case QUEEN:
                    *square = 'q';
                    break;
                }

                // Capitalize the letters of the white pieces
                if ((piece & COLOR_MASK) == WHITE)
                {
                    *square = *square + ('A' - 'a');
                }
            }
            
            square++;
            *square = '|';
            square++;
        }

        *square = '\0';

        printf("|%s\n", line);
    }
}

void printMove(Move *p_move)
{
    uint8_t f_rank = p_move->from / 8;
    uint8_t f_file = p_move->from % 8;

    uint8_t t_rank = p_move->to / 8;
    uint8_t t_file = p_move->to % 8;

    printf("%c%d -> %c%d\n", 'a' + f_file, f_rank + 1, 'a' + t_file, t_rank + 1);
}

void printMoveList(ArrayList *p_list)
{
    for(uint16_t i = 0; i < p_list->elements; i++)
    {
        printf("(%d)\t", i);
        printMove(&p_list->array[i]);
    }
}

// Lists all the available moves and prompts the user to select a move
Move selectMove(Board *p_board)
{
    ArrayList *p_legalMoves = getLegalMoves(p_board);
    printMoveList(p_legalMoves);

    unsigned int index = p_legalMoves->elements;
    while (index >= p_legalMoves->elements)
    {
        printf("Select move: \n");
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-result"
        scanf("%u", &index);
        fgetc(stdin); // Empty the stdin buffer
        #pragma GCC diagnostic pop
    }

    Move move = p_legalMoves->array[index];
    freeMoveList(p_legalMoves);

    return move;
}