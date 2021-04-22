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

void m_playComputerTurn(Board *p_board, uint8_t ply);
void m_playUserTurn(Board *p_board);

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
    //__test__evaluation();

    Board board;
    Board *p_board = &board;
    createBoardFormFEN(INITIAL_BOARD, p_board);
    initEvalTables();

    int8_t boardStatus;

    printBoard(p_board);
    for (uint8_t i = 0; i < 100; i++)
    {
        printf("Move number: %d\n", p_board->fullMoves);
        
        m_playComputerTurn(p_board, 4);
        printf("EVAL: %ld\n", evaluateBoard(p_board, LEGAL_MOVES_EXIST, WHITE));
        //m_playUserTurn(p_board);

        boardStatus = isCheckmate(p_board);
        if (boardStatus)
            break;

        //m_playComputerTurn(p_board, 4);
        m_playUserTurn(p_board);
        printf("EVAL: %ld\n", evaluateBoard(p_board, LEGAL_MOVES_EXIST, WHITE));

        boardStatus = isCheckmate(p_board);
        if (boardStatus)
            break;
    }

    if (boardStatus == 1)
    {
        printf("Stalemate!\n");
    }
    else if (boardStatus == WHITE)
    {
        printf("White won!\n");
    }
    else if (boardStatus == BLACK)
    {
        printf("Black won!\n");
    }
    else
    {
        printf("Game terminated!");
    }

    printf("Exit success\n");
    return 0;
}

void m_playComputerTurn(Board *p_board, uint8_t ply)
{
    Move bestMove = findBestMove(p_board, ply);
    performMove(&bestMove, p_board);
    printBoard(p_board);
}

void m_playUserTurn(Board *p_board)
{
    Move slectedMove = selectMove(p_board);
    performMove(&slectedMove, p_board);
    printBoard(p_board);
}