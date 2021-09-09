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
#include "book.h"

// #include "test.h"

void m_playComputerTurn(Board *p_board, Book *book, uint8_t ply);
void m_playUserTurn(Board *p_board, Book *book);

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

    Book book;

    printf("Generating book...\n");
    clock_t start = clock(), diff;

    generateBook(&book, 0, "uci.txt");
    book.status = BOOK_ENDED;

    diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Book generation time: %d sec, %d ms\n", msec / 1000, msec % 1000);

    Board board;
    Board *p_board = &board;
    createBoardFormFEN(INITIAL_BOARD, p_board);
    initEvalTables();

    int8_t boardStatus;

    printBoard(p_board);
    for (uint8_t i = 0; i < 20; i++)
    {
        printf("Move number: %d\n", i + 1);
        m_playComputerTurn(p_board, &book, 6);
        //m_playUserTurn(p_board, &book);
        printf("Book status: %s\n", book.status == BOOK_READY ? "in" : "out");

        boardStatus = isCheckmate(p_board);
        if (boardStatus)
            break;

        m_playComputerTurn(p_board, &book, 6);
        //m_playUserTurn(p_board, &book);
        printf("Book status: %s\n", book.status == BOOK_READY ? "in" : "out");

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

    freeBook(&book);
    extern Hashmap *p_tt;
    freehashmap(p_tt);

    printf("Exit success\n");
    return 0;
}

void m_playComputerTurn(Board *p_board, Book *book, uint8_t ply)
{
    static clock_t totalTime;
    static uint16_t numComputerMoves;

    clock_t start = clock(), diff;

    Move bestMove;
    if (book->status == BOOK_READY)
    {
        getNextMove(book, &bestMove);
    }
    else
    {
        bestMove = findBestMove(p_board, ply);
    }

    diff = clock() - start;
    totalTime += diff;
    numComputerMoves++;
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    int totalMS = totalTime * 1000 / (CLOCKS_PER_SEC * numComputerMoves);
    printf("Move calculation time: %d sec, %d ms\n", msec / 1000, msec % 1000);
    printf("Average time: %d sec, %d ms\n", totalMS / 1000, totalMS % 1000);
    performMove(&bestMove, p_board);
    printBoard(p_board);
}

void m_playUserTurn(Board *p_board, Book *book)
{
    Move selectedMove = selectMove(p_board);
    advanceInBook(book, selectedMove);
    performMove(&selectedMove, p_board);
    printBoard(p_board);
}