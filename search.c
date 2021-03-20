#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "moveHandler.h"
#include "search.h"
#include "eval.h"
#include "sorting.h"
#include "hashing.h"

#define INF 9223372036854775807

int64_t m_alphabeta(Board *p_board, uint8_t depth, evaluation_t alpha, evaluation_t beta, uint8_t maximizer);
int64_t m_alphaBetaCaptures(Board *p_board, evaluation_t alpha, evaluation_t beta);
uint16_t m_movePriority(Move *p_move, Board *p_board);

Hashmap *p_tt;
uint32_t leafNodesEvaluated = 0;
uint32_t transpositionHits = 0;

Move findBestMove(Board *p_board, uint8_t depth)
{
    leafNodesEvaluated = 0;
    transpositionHits = 0;
    p_tt = createHashmap(1024);
    List *p_legalMoves = getLegalMoves(p_board);
    sort(p_legalMoves, p_board, m_movePriority);
    Board tmpBoard;
    Node *p_moveNode = p_legalMoves->p_head;

    Move bestMove;

    int64_t alpha = -INF;
    int64_t beta = INF;
    int64_t value = -INF;
    for (uint8_t i = 0; i < p_legalMoves->length; i++)
    {
        performMove(p_moveNode->p_move, p_board);
        int64_t tmpValue = m_alphabeta(p_board, depth - 1, alpha, beta, 0);
        undoMove(p_moveNode->p_move, p_board);

        // select max and set new best move
        if (tmpValue > value)
        {
            value = tmpValue;
            bestMove = *(p_moveNode->p_move);
        }

        alpha = fmaxl(alpha, value);
        if (alpha >= beta)
        {
            appendToHashmap(p_tt, p_board, beta, depth, *p_moveNode->p_move, LOWER_BOUND);
            break;
        }

        p_moveNode = p_moveNode->p_next;
    }

    freeMoveList(p_legalMoves);
    freehashmap(p_tt);

    printf("Evaluation: %ld\n", value);
    printf("Leafnodes evaluated: %d\n", leafNodesEvaluated);
    printf("Transpositions hit: %d\n", transpositionHits);

    return bestMove;
}

int64_t m_alphabeta(Board *p_board, uint8_t depth, int64_t alpha, int64_t beta, uint8_t maximizer)
{
    int64_t transpositionLookup = getEvaluation(p_tt, p_board, depth, alpha, beta);
    if (transpositionLookup != EVAL_LOOKUP_FAILED)
    {
        transpositionHits++;
        return transpositionLookup;
    }

    List *p_legalMoves = getLegalMoves(p_board);

    if (depth == 0)
    {
        leafNodesEvaluated++;
        return evaluateBoard(p_board, 0); //m_alphaBetaCaptures(p_board, alpha, beta);
    }

    // Here we also want to handle potential move repetitions
    // If the depth is reach or there are no legal moves (checkmate or stalemate)
    if (p_legalMoves->length == 0)
    {
        leafNodesEvaluated++;
        freeMoveList(p_legalMoves);
        return evaluateBoard(p_board, 1);
    }

    sort(p_legalMoves, p_board, m_movePriority);
    uint8_t wasCut = 0;
    int64_t value;
    Node *p_moveNode = p_legalMoves->p_head;
    if (maximizer)
    {
        value = -INF;
        for (uint8_t i = 0; i < p_legalMoves->length; i++)
        {
            performMove(p_moveNode->p_move, p_board);
            value = fmaxl(value, m_alphabeta(p_board, depth - 1, alpha, beta, 0));
            undoMove(p_moveNode->p_move, p_board);

            alpha = fmaxl(alpha, value);
            if (alpha >= beta)
            {
                appendToHashmap(p_tt, p_board, beta, depth, *p_moveNode->p_move, LOWER_BOUND);
                wasCut = 1;
                break;
            }

            p_moveNode = p_moveNode->p_next;
        }
    }
    else
    {
        value = INF;
        for (uint8_t i = 0; i < p_legalMoves->length; i++)
        {
            performMove(p_moveNode->p_move, p_board);
            value = fminl(value, m_alphabeta(p_board, depth - 1, alpha, beta, 1));
            undoMove(p_moveNode->p_move, p_board);

            beta = fminl(beta, value);
            if (beta <= alpha)
            {
                appendToHashmap(p_tt, p_board, alpha, depth, *p_moveNode->p_move, UPPER_BOUND);
                wasCut = 1;
                break;
            }

            p_moveNode = p_moveNode->p_next;
        }
    }

    if (!wasCut)
    {
        Move randomMove;
        appendToHashmap(p_tt, p_board, value, depth, randomMove, EXACT);
    }

    freeMoveList(p_legalMoves);
    return value;
}

int64_t m_alphaBetaCaptures(Board *p_board, int64_t alpha, int64_t beta)
{
    // Get the evaluation of the board, to compair the position
    // without captures to the positions to with captures
    List *p_legalMoves = getLegalMoves(p_board);
    int64_t eval = evaluateBoard(p_board, p_legalMoves->length == 0);

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
    sort(p_legalMoves, p_board, m_movePriority);

    Node *p_node = p_legalMoves->p_head;
    while (p_node != NULL)
    {
        performMove(p_node->p_move, p_board);
        eval = m_alphaBetaCaptures(p_board, -beta, -alpha);
        undoMove(p_node->p_move, p_board);

        if (eval >= beta)
        {
            freeMoveList(p_legalMoves);
            return beta;
        }
        if (eval > alpha)
        {
            alpha = eval;
        }

        p_node = p_node->p_next;
    }

    freeMoveList(p_legalMoves);

    return alpha;
}

uint16_t m_movePriority(Move *p_move, Board *p_board)
{
    uint16_t captureValue = 0;
    switch (p_move->capture & TYPE_MASK)
    {
    case PAWN:
        captureValue = PAWN_VALUE;
        break;
    case ROOK:
        captureValue = ROOK_VALUE;
        break;
    case KNIGHT:
        captureValue = KNIGHT_VALUE;
        break;
    case BISHOP:
        captureValue = BISHOP_VALUE;
        break;
    case QUEEN:
        captureValue = QUEEN_VALUE;
        break;
    }

    uint16_t capturingValue = 0;
    switch (p_board->board[p_move->from] & TYPE_MASK)
    {
    case PAWN:
        capturingValue = PAWN_VALUE;
        break;
    case ROOK:
        capturingValue = ROOK_VALUE;
        break;
    case KNIGHT:
        capturingValue = KNIGHT_VALUE;
        break;
    case BISHOP:
        capturingValue = BISHOP_VALUE;
        break;
    case QUEEN:
        capturingValue = QUEEN_VALUE;
        break;
    case KING:
        capturingValue = KING_VALUE;
        break;
    }

    // 10 is just chosen to make the focus on what is captured and not what is capturing it
    // Negative because we want the high priorities first
    return -(captureValue * 20 - capturingValue);
}