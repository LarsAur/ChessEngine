#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"
#include "fen.h"
#include "utils.h"
#include "moveHandler.h"
#include "search.h"
#include "hashing.h"

#include "test.h"

int main(void)
{
    //__test__FEN();
    //__test__pawnMovement();
    //__test__slidingMovement();
    //__test__kingMovement();
    //__test__knightMovement();
    __test__moveTree();
    __test__hashmap();

    // srand((unsigned)time(NULL));
    // printf("SEED: %d\n", (unsigned)time(NULL));

    // Board *p_board = malloc(sizeof(Board));
    // createBoardFormFEN(INITIAL_BOARD, p_board);

    // for (uint8_t i = 0; i < 250; i++)
    // {
    //     List *p_legalMoveList = getLegalMoves(p_board);

    //     if (p_legalMoveList->length == 0)
    //     {
    //         printf("%s Won\n", p_board->turn == WHITE ? "BLACK" : "WHITE");
    //         break;
    //     }

    //     Node *p_node = p_legalMoveList->p_head;
        
    //     /*printf("Select move: \n");
    //     printMoveList(p_legalMoveList);
    //     unsigned int index = p_legalMoveList->length + 1;
    //     while (index > p_legalMoveList->length)
    //     {
    //         int len = scanf("%u", &index);
    //     }*/
        
    //     uint8_t index = rand() % p_legalMoveList->length;
    //     p_node = p_legalMoveList->p_head;
        
    //     for (uint8_t j = 0; j < index; j++)
    //     {
    //         p_node = p_node->p_next;
    //     }

    //     performMove(p_node->p_move, p_board);
    //     freeMoveList(p_legalMoveList);

    //     printBoard(p_board);
    //     printf("------%s-----\n", p_board->turn == WHITE ? "WHITE" : "BLACK");

    //     Move bestMove = findBestMove(p_board, 6);

    //     printf("Found best move: ");
    //     printMove(&bestMove);

    //     performMove(&bestMove, p_board);
    //     printBoard(p_board);
    //     printf("------%s-----\n", p_board->turn == WHITE ? "WHITE" : "BLACK");
    // }

    // free(p_board);
    printf("Exit success\n");
    return 0;
}