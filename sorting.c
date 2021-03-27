#include <stdio.h>
#include <stdlib.h>

#include "sorting.h"

void m_mergeSort(Node **headRef, Board *p_board, uint16_t (*orderEval)(Move *, Board *));
Node *m_sortedMerge(Node *a, Node *b, Board *p_board, uint16_t (*orderEval)(Move *, Board *));
void m_split(Node *src, Node **frontRef, Node **backRef);

void sort(List *p_list, Board *p_board, uint16_t (*orderEval)(Move *, Board *))
{
    m_mergeSort(&p_list->p_head, p_board, orderEval);
}

void m_mergeSort(Node **headRef, Board *p_board, uint16_t (*orderEval)(Move *, Board *))
{
    Node *p_head = *headRef;
    Node *a;
    Node *b;

    // One element and an empty list can only have one permutation
    if ((p_head == NULL) || (p_head->p_next == NULL))
    {
        return;
    }

    m_split(p_head, &a, &b);

    m_mergeSort(&a, p_board, orderEval);
    m_mergeSort(&b, p_board, orderEval);

    *headRef = m_sortedMerge(a, b, p_board, orderEval);
}

Node *m_sortedMerge(Node *a, Node *b, Board *p_board, uint16_t (*orderEval)(Move *, Board *))
{
    Node *result = NULL;

    /* Base cases */
    if (a == NULL)
        return (b);
    else if (b == NULL)
        return (a);

    /* Pick either a or b, and recur */
    if (orderEval(a->p_move, p_board) <= orderEval(b->p_move, p_board))
    {
        result = a;
        result->p_next = m_sortedMerge(a->p_next, b, p_board, orderEval);
    }
    else
    {
        result = b;
        result->p_next = m_sortedMerge(a, b->p_next, p_board, orderEval);
    }
    
    return result;
}

void m_split(Node *src, Node **frontRef, Node **backRef)
{
    Node *fast;
    Node *slow;
    slow = src;
    fast = src->p_next;

    // Advance 'fast' two nodes, and advance 'slow' one node
    while (fast != NULL)
    {
        fast = fast->p_next;
        if (fast != NULL)
        {
            slow = slow->p_next;
            fast = fast->p_next;
        }
    }

    /* 'slow' is before the midpoint in the list, so split it in two 
    at that point. */
    *frontRef = src;
    *backRef = slow->p_next;
    slow->p_next = NULL;
}