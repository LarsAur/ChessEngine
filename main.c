#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "main.h"
#include "fen.h"
#include "utils.h"
#include "moveHandler.h"
#include "search.h"
#include "hashing.h"
#include "eval.h"

#include "test.h"

int main(void)
{
    //__test__FEN();
    //__test__pawnMovement();
    //__test__slidingMovement();
    //__test__kingMovement();
    //__test__knightMovement();
    //__test__moveTree();
    //__test__hashmap();
    //__test__checkmate();

    //srand(1616866069);
    //srand((unsigned)time(NULL));
    //printf("SEED: %d\n", (unsigned)time(NULL));

    Board board;
    Board *p_board = &board;
    createBoardFormFEN(INITIAL_BOARD, p_board);

    for (uint8_t i = 0; i < 250; i++)
    {

        /*if (p_legalMoveList->length == 0)
        {
            printf("%s Won\n", p_board->turn == WHITE ? "BLACK" : "WHITE");
            break;
        }*/

        
        /*uint8_t index = rand() % p_legalMoveList->length;
        p_node = p_legalMoveList->p_head;
        


        performMove(p_node->p_move, p_board);
        freeMoveList(p_legalMoveList);*/

        //printBoard(p_board);
        //printf("------%s-----\n", p_board->turn == WHITE ? "WHITE" : "BLACK");
        
        initEvalTables();
        
        Move bestMove = findBestMove(p_board, 4);
        printf("Move number: %d\n", p_board->fullMoves);
        printf("Found best move: ");
        printMove(&bestMove);
        performMove(&bestMove, p_board);
        printBoard(p_board);
        printf("------%s-----\n", p_board->turn == WHITE ? "WHITE" : "BLACK");

        List *p_legalMoveList = getLegalMoves(p_board);
        Node *p_node = p_legalMoveList->p_head;
        
        printMoveList(p_legalMoveList);
        unsigned int index = p_legalMoveList->length + 1;
        while (index >= p_legalMoveList->length)
        {
            printf("Select move: \n");
            int len = scanf("%u", &index);
        }
        
        printf("Index %d", index);

        for (uint8_t j = 0; j < index; j++)
        {
            p_node = p_node->p_next;
        }

        performMove(p_node->p_move, p_board);
        freeMoveList(p_legalMoveList);
        printBoard(p_board);
        printf("------%s-----\n", p_board->turn == WHITE ? "WHITE" : "BLACK");
        
    }

    printf("Exit success\n");
    return 0;
}