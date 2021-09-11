#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "main.h"
#include "moveHandler.h"
#include "search.h"
#include "eval.h"
#include "sorting.h"
#include "hashing.h"
#include "utils.h"

#define INF 9223372036854775807

int64_t m_alphabeta(Board *p_board, uint8_t depth, evaluation_t alpha, evaluation_t beta, uint8_t maximizer);
int64_t m_alphaBetaCaptures(Board *p_board, evaluation_t alpha, evaluation_t beta);
uint8_t m_numBoardRepeates(Board *p_board);

Hashmap *p_tt = NULL;
uint32_t leafNodesEvaluated = 0;
uint32_t transpositionHits = 0;
uint8_t color = 0; //Color of the player to find the best move for, this is set in findbestmove and passed to evaluation

Move findBestMove(Board *p_board, uint8_t depth)
{
    color = p_board->turn;
    leafNodesEvaluated = 0;
    transpositionHits = 0;
    if(!p_tt) p_tt = createHashmap(1 << 20);
    ArrayList *p_legalMoves = getLegalMoves(p_board);
    sort(p_legalMoves, p_board);

    Move bestMove;

    int64_t alpha = -INF;
    int64_t beta = INF;
    int64_t value = -INF;
    for (uint8_t i = 0; i < p_legalMoves->elements; i++)
    {
        performMove(&p_legalMoves->array[i], p_board);
        int64_t tmpValue = m_alphabeta(p_board, depth - 1, alpha, beta, 0);
        undoMove(&p_legalMoves->array[i], p_board);

        // select max and set new best move
        if (tmpValue > value)
        {
            value = tmpValue;
            bestMove = p_legalMoves->array[i];
        }

        alpha = fmaxl(alpha, value);
        if (alpha >= beta)
        {
            appendToHashmap(p_tt, p_board, beta, depth, LOWER_BOUND);
            break;
        }
    }

    freeMoveList(p_legalMoves);
    //freehashmap(p_tt);

    printf("Evaluation: %ld\n", value);
    printf("Leafnodes evaluated: %d\n", leafNodesEvaluated);
    printf("Transpositions hit: %d\n", transpositionHits);
    printf("Table Size: %ld\n", p_tt->size);
    printf("Phase: %d (0 = opening)\n", getPhase(p_board));
    printf("Best move: ");
    printMove(&bestMove);

    return bestMove;
}

int64_t m_alphabeta(Board *p_board, uint8_t depth, int64_t alpha, int64_t beta, uint8_t maximizer)
{
    if(m_numBoardRepeates(p_board) == 2)
    {
        return STALEMATE;
    }

    int64_t transpositionLookup = getEvaluation(p_tt, p_board, depth, alpha, beta);

    if (transpositionLookup != EVAL_LOOKUP_FAILED)
    {
        transpositionHits++;
        return transpositionLookup;
    }

    // 50 move rule makes a draw
    if (p_board->halfMoves == 50)
    {
        return 0;
    }

    ArrayList *p_legalMoves = getLegalMoves(p_board);

    // If the depth is reach or there are no legal moves (checkmate or stalemate)
    if (p_legalMoves->elements == 0)
    {
        leafNodesEvaluated++;
        freeMoveList(p_legalMoves);
        evaluation_t terminationEval = evaluateBoard(p_board, NO_LEGAL_MOVES, color);

        if(terminationEval == STALEMATE)
        {
            return terminationEval;
        }

        // This makes a checkmate at a shorter depth more attractive
        // For the opponent a shorter checkmate is also less attractive
        return terminationEval * (1 + depth);
    }

    if (depth == 0)
    {
        leafNodesEvaluated++;
        freeMoveList(p_legalMoves);
        return evaluateBoard(p_board, LEGAL_MOVES_EXIST, color); /*-m_alphaBetaCaptures(p_board, -beta, -alpha);*/
    }

    sort(p_legalMoves, p_board);
    uint8_t wasCut = 0;
    int64_t value;
    if (maximizer)
    {
        value = -INF;
        for (uint8_t i = 0; i < p_legalMoves->elements; i++)
        {
            performMove(&p_legalMoves->array[i], p_board);
            value = fmaxl(value, m_alphabeta(p_board, depth - 1, alpha, beta, 0));
            undoMove(&p_legalMoves->array[i], p_board);

            alpha = fmaxl(alpha, value);
            if (alpha >= beta)
            {
                appendToHashmap(p_tt, p_board, beta, depth, LOWER_BOUND);
                wasCut = 1;
                break;
            }
        }
    }
    else
    {
        value = INF;
        for (uint8_t i = 0; i < p_legalMoves->elements; i++)
        {
            performMove(&p_legalMoves->array[i], p_board);
            value = fminl(value, m_alphabeta(p_board, depth - 1, alpha, beta, 1));
            undoMove(&p_legalMoves->array[i], p_board);

            beta = fminl(beta, value);
            if (beta <= alpha)
            {
                appendToHashmap(p_tt, p_board, alpha, depth, UPPER_BOUND);
                wasCut = 1;
                break;
            }
        }
    }

    if (!wasCut)
    {
        appendToHashmap(p_tt, p_board, value, depth, EXACT);
    }

    freeMoveList(p_legalMoves);
    return value;
}

int64_t m_alphaBetaCaptures(Board *p_board, int64_t alpha, int64_t beta)
{
    // Get the evaluation of the board, to compair the position
    // without captures to the positions to with captures
    ArrayList *p_legalMoves = getLegalMoves(p_board);
    int64_t eval = evaluateBoard(p_board, p_legalMoves->elements ? LEGAL_MOVES_EXIST : NO_LEGAL_MOVES, color);

    if (eval >= beta)
    {
        freeMoveList(p_legalMoves);
        return beta;
    }
    if (eval > alpha)
    {
        alpha = eval;
    }

    filterNonCaptureMoves(p_legalMoves);
    sort(p_legalMoves, p_board);

    for(uint16_t i = 0; i < p_legalMoves->elements; i++)
    {
        performMove(&p_legalMoves->array[i], p_board);
        eval = m_alphaBetaCaptures(p_board, -beta, -alpha);
        undoMove(&p_legalMoves->array[i], p_board);

        if (eval >= beta)
        {
            freeMoveList(p_legalMoves);
            return beta;
        }
        if (eval > alpha)
        {
            alpha = eval;
        }
    }

    freeMoveList(p_legalMoves);

    return alpha;
}

uint8_t m_numBoardRepeates(Board *p_board)
{
    uint16_t tailIndex = (p_board->fullMoves - 1) * 2 + (p_board->turn == BLACK);
    uint8_t repeats = 0;
    for (uint16_t i = 0; i < tailIndex; i++)
    {
        repeats += p_board->gameHashHistory[i] == p_board->hash;
    }
    return repeats;
}