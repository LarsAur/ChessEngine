#include <stdio.h>
#include <stdlib.h>

#include "eval.h"
#include "sorting.h"

int16_t m_partition(Move arr[], int16_t low, int16_t high, Board *p_board);
void m_quickSort(Move arr[], int16_t low, int16_t high, Board *p_board);
int16_t m_movePriority(Move *p_move, Board *p_board);

void sort(ArrayList *p_list, Board *p_board)
{
    m_quickSort(p_list->array, 0, p_list->elements - 1, p_board);
}

void m_swap(Move *a, Move *b)
{
    Move t = *a;
    *a = *b;
    *b = t;
}

void m_quickSort(Move arr[], int16_t low, int16_t high, Board *p_board)
{
    if(low < high)
    {
        int16_t pivotIndex = m_partition(arr, low, high, p_board);

        m_quickSort(arr, low, pivotIndex - 1, p_board);
        m_quickSort(arr, pivotIndex + 1, high, p_board);
    }
}

int16_t m_partition(Move arr[], int16_t low, int16_t high, Board *p_board)
{
    int16_t pivot = m_movePriority(&arr[high], p_board);
    int16_t i = low - 1;

    for(int16_t j = low; j < high; j++)
    {
        if(m_movePriority(&arr[j], p_board) < pivot)
        {
            i++;
            m_swap(&arr[i], &arr[j]);
        }
    }
    m_swap(&arr[i + 1], &arr[high]);
    return i + 1;
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

int16_t m_movePriority(Move *p_move, Board *p_board)
{
    uint16_t captureValue = piecePriority[p_board->board[p_move->to] & TYPE_MASK];
    uint16_t capturingValue = piecePriority[p_board->board[p_move->from] & TYPE_MASK];

    // 20 is just chosen to make the focus on what is captured and not what is capturing it
    // Negative because we want the high priorities first
    return -(captureValue * 20 + capturingValue);
}