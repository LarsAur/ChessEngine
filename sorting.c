#include <stdio.h>
#include <stdlib.h>

#include "sorting.h"

uint16_t m_partition(Move arr[], uint16_t low, uint16_t high, Board *p_board, uint16_t (*orderEval)(Move *, Board *));
void m_quickSort(Move arr[], uint16_t low, uint16_t high, Board *p_board, uint16_t (*orderEval)(Move *, Board *));

void sort(ArrayList *p_list, Board *p_board, uint16_t (*orderEval)(Move *, Board *))
{
    m_quickSort(p_list->array, 0, p_list->elements - 1, p_board, orderEval);
}

void m_quickSort(Move arr[], uint16_t low, uint16_t high, Board *p_board, uint16_t (*orderEval)(Move *, Board *))
{
    if(low < high)
    {
        uint16_t pivotIndex = m_partition(arr, low, high, p_board, orderEval);

        m_quickSort(arr, low, pivotIndex - 1, p_board, orderEval);
        m_quickSort(arr, pivotIndex, high, p_board, orderEval);
    }
}

uint16_t  m_partition(Move arr[], uint16_t low, uint16_t high, Board *p_board, uint16_t (*orderEval)(Move *, Board *))
{
    uint16_t pivot = orderEval(&arr[high], p_board);
    uint16_t i = low - 1;

    for(uint16_t j = low; j < high; j++)
    {
        if(orderEval(&arr[j], p_board) < pivot)
        {
            i++;
            Move tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }
    }

    Move tmp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = tmp;
    return i + 1;
}