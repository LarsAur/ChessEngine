#include "emscripten.h"
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "fen.h"
#include "moveHandler.h"
#include "book.h"
#include "search.h"
#include "utils.h"

Board board;
Book book;

void EMSCRIPTEN_KEEPALIVE init(int book_depth)
{
    printf("%p\n", &board);
    initEvalTables();
    generateBook(&book, book_depth, "uci.txt");
}

void EMSCRIPTEN_KEEPALIVE resetBoard()
{
    createBoardFormFEN(INITIAL_BOARD, &board);
    book.currentNode = book.initialNode;
    book.status = BOOK_READY;
}

int EMSCRIPTEN_KEEPALIVE getCurrentBoardPointer()
{
    return (int)&board;
}

int EMSCRIPTEN_KEEPALIVE getCurrentBoardSize()
{
    return (int)sizeof(Board);
}

int EMSCRIPTEN_KEEPALIVE isBoardCheckmate()
{
    return isCheckmate(&board);
}

int EMSCRIPTEN_KEEPALIVE getTurn()
{
    return board.turn;
}

int EMSCRIPTEN_KEEPALIVE performMatchingMove(int from, int to, int promotion)
{
    List *p_legalMoves = getLegalMoves(&board);

    Node *p_node = p_legalMoves->p_head;
    while (p_node != (void *)0 && !(p_node->p_move->from == from && p_node->p_move->to == to && p_node->p_move->promotion == promotion))
    {
        p_node = p_node->p_next;
    }

    if (p_node == (void *)0)
    {
        return 0;
    }

    if(book.status == BOOK_READY)
    {
        printf("1 READY");
    }
    else
    {
        printf("1 ENDED");
    }

    printMove(p_node->p_move);
    advanceInBook(&book, *p_node->p_move);
    
    if(book.status == BOOK_READY)
    {
        printf("2 READY");
    }
    else
    {
        printf("2 ENDED");
    }

    performMove(p_node->p_move, &board);
    freeMoveList(p_legalMoves);
    return 1;
}

void EMSCRIPTEN_KEEPALIVE playAIMove(int ply)
{
    Move move;
    if (book.status == BOOK_ENDED)
    {
        printf("3 ENDED");
        move = findBestMove(&board, ply);
    }
    else
    {
        printf("3 READY");
        getNextMove(&book, &move);
    }

    performMove(&move, &board);
}