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
#include "utils.h"

#define INF 9223372036854775807

int64_t m_alphabeta(Board *p_board, uint8_t depth, evaluation_t alpha, evaluation_t beta, uint8_t maximizer);
int64_t m_alphaBetaCaptures(Board *p_board, evaluation_t alpha, evaluation_t beta);
uint16_t m_movePriority(Move *p_move, Board *p_board);
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
    if(!p_tt) p_tt = createHashmap(2 << 20);
    List *p_legalMoves = getLegalMoves(p_board);
    sort(p_legalMoves, p_board, m_movePriority);
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
            appendToHashmap(p_tt, p_board, beta, depth, LOWER_BOUND);
            break;
        }

        p_moveNode = p_moveNode->p_next;
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
        return 0;
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

    List *p_legalMoves = getLegalMoves(p_board);

    // If the depth is reach or there are no legal moves (checkmate or stalemate)
    if (p_legalMoves->length == 0)
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
                appendToHashmap(p_tt, p_board, beta, depth, LOWER_BOUND);
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
                appendToHashmap(p_tt, p_board, alpha, depth, UPPER_BOUND);
                wasCut = 1;
                break;
            }

            p_moveNode = p_moveNode->p_next;
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
    List *p_legalMoves = getLegalMoves(p_board);
    int64_t eval = evaluateBoard(p_board, p_legalMoves->length ? LEGAL_MOVES_EXIST : NO_LEGAL_MOVES, color);

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

uint16_t piecePriority[7] =
{
    EMPTY,
    PAWN_VALUE,
    ROOK_VALUE,
    KNIGHT_VALUE,
    BISHOP_VALUE,
    QUEEN_VALUE,
    KING_VALUE,
};

uint16_t m_movePriority(Move *p_move, Board *p_board)
{
    uint16_t captureValue = piecePriority[p_move->capture & TYPE_MASK];
    uint16_t capturingValue = piecePriority[p_board->board[p_move->from] & TYPE_MASK];

    // 10 is just chosen to make the focus on what is captured and not what is capturing it
    // Negative because we want the high priorities first
    return -(captureValue * 20 - capturingValue);
}