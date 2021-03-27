#include <stdlib.h>
#include <stdio.h>
#include "hashing.h"
#include "main.h"
#include "moveHandler.h"

void m_initZobrist();
hash_t zobristHash(Board *p_board);

Hashmap *createHashmap(uint64_t numBuckets)
{
    Hashmap *p_hashmap = malloc(sizeof(Hashmap));
    p_hashmap->numBuckets = numBuckets;

    Bucket **buckets = malloc(numBuckets * sizeof(Bucket *));
    for (uint64_t i = 0; i < numBuckets; i++)
    {
        buckets[i] = NULL;
    }

    p_hashmap->buckets = buckets;

    return p_hashmap;
}

void freehashmap(Hashmap *p_hashmap)
{
    // Loop through each bucket and free all of the buckets
    for (uint64_t i = 0; i < p_hashmap->numBuckets; i++)
    {
        Bucket *bucket = p_hashmap->buckets[i];
        Bucket *tmp = bucket->p_next;

        // Loop through all of the buckets with the same hash and free them
        while (bucket != NULL)
        {
            tmp = bucket->p_next;
            free(bucket);
            bucket = tmp;
        }
    }

    free(p_hashmap->buckets);
    free(p_hashmap);
}

// Adds the the board with the evaluation to the hashmap
void appendToHashmap(Hashmap *p_hashmap, Board *p_board, evaluation_t eval, uint8_t depth, Move move, EvalType evalType)
{
    hash_t hash = p_board->hash;
    uint64_t bucketId = hash % p_hashmap->numBuckets; // Fit hash into the number of buckets

    Bucket *p_newBucket = malloc(sizeof(Bucket));
    p_newBucket->hash = hash;
    p_newBucket->evalScore = eval;
    p_newBucket->depth = depth;
    p_newBucket->evalType = evalType;
    p_newBucket->move = move;

    // Put the new bucket element at the front of the linked list
    p_newBucket->p_next = p_hashmap->buckets[bucketId];
    p_hashmap->buckets[bucketId] = p_newBucket;
}

// Checks if the board exists in the hashmap
uint8_t existsInHashmap(Hashmap *p_hashmap, Board *p_board)
{
    hash_t hash = p_board->hash;
    uint64_t bucketId = hash % p_hashmap->numBuckets;

    Bucket *bucket = p_hashmap->buckets[bucketId];
    while (bucket != NULL)
    {
        if (hash == bucket->hash)
        {
            // The board was found
            return 1;
        }
        bucket = bucket->p_next;
    }

    // The board was not found
    return 0;
}

// Gets the evaluation of the board, this function assums that the board is in the hashmap
evaluation_t getEvaluation(Hashmap *p_hashmap, Board *p_board, uint8_t depth, int64_t alpha, int64_t beta)
{
    hash_t hash = p_board->hash;
    uint64_t bucketId = hash % p_hashmap->numBuckets;

    Bucket *bucket = p_hashmap->buckets[bucketId];
    while (bucket != NULL)
    {
        if (hash == bucket->hash)
        {
            if (bucket->depth >= depth)
            {
                // The board was found
                if (bucket->evalType == EXACT)
                    return bucket->evalScore;

                if (bucket->evalType == UPPER_BOUND && bucket->evalScore <= alpha)
                    return bucket->evalScore;

                if (bucket->evalType == LOWER_BOUND && bucket->evalScore >= beta)
                    return bucket->evalScore;
            }
        }
        bucket = bucket->p_next;
    }

    return EVAL_LOOKUP_FAILED;
}

uint64_t table[12][64];
uint8_t initialized = 0;
hash_t zobristHash(Board *p_board)
{
    if (!initialized)
    {
        m_initZobrist();
    }

    hash_t hash = 0;
    for (uint8_t i = 0; i < 64; i++)
    {
        if (p_board->board[i] != EMPTY)
        {
            // White pieces are 1-6 and black pieces are 7-12 (inclusive)
            uint8_t j = (p_board->board[i] & TYPE_MASK) + 6 * ((p_board->board[i] & COLOR_MASK) == BLACK);
            hash ^= table[j][i];
        }
    }

    hash ^= p_board->enPassantTarget | p_board->castleRights << 8 | p_board->turn << 10;

    return hash;
}

// Recieves the new board with the move that was performed previously
// The new hash value of the board is set and returned
hash_t updateZobristHash(Board *p_board, Move *p_move)
{
    // If casteling the rook have to be xor'ed in and out at the correct positions
    if(p_move->castle)
    {
        // Queenside castle
        if(p_move->to < p_move->from)
        {
            // XOR out the rook
            uint8_t j = ROOK + 6 * ((p_board->board[p_move->to - 2] & COLOR_MASK) == BLACK);
            p_board->hash ^= table[j][p_move->to - 2];
            // XOR in the rook
            p_board->hash ^= table[j][p_move->to + 1];
        }
        // Kingside castle
        else
        {
            // XOR out the rook
            uint8_t j = ROOK + 6 * ((p_board->board[p_move->to + 1] & COLOR_MASK) == BLACK);
            p_board->hash ^= table[j][p_move->to + 1];
            // XOR in the rook
            p_board->hash ^= table[j][p_move->to - 1];
        }
    }

    // XOR out the moved piece from initial square, this is also XOR'ing in an empty square
    // If the move was a promotion, a pawn has to be XOR'ed out
    uint8_t j;
    if(p_move->promotion)
    {
        j = PAWN + 6 * ((p_board->board[p_move->to] & COLOR_MASK) == BLACK);
    }
    else
    {
        j = (p_board->board[p_move->to] & TYPE_MASK) + 6 * ((p_board->board[p_move->to] & COLOR_MASK) == BLACK);
    }
    p_board->hash ^= table[j][p_move->from];

    // XOR in the moved piece at destination
    j = (p_board->board[p_move->to] & TYPE_MASK) + 6 * ((p_board->board[p_move->to] & COLOR_MASK) == BLACK);
    p_board->hash ^= table[j][p_move->to];

    // XOR out the possible captured piece
    if (p_move->capture != EMPTY)
    {
        if (p_move->enPassant)
        {
            // If black captured the piece (ie. it is whites turn)
            // Remove the piece from the double move square above the capture
            // if the capturer was white, the pawn was below the capture position
            uint8_t i = p_move->to + (p_board->turn == WHITE ? 8 : -8);
            uint8_t j = PAWN + 6 * ((p_move->capture & COLOR_MASK) == BLACK);
            p_board->hash ^= table[j][i];
        }
        else
        {
            // Remove the captured piece which was positioned at the capute point
            uint8_t i = p_move->to;
            uint8_t j = (p_move->capture & TYPE_MASK) + 6 * ((p_move->capture & COLOR_MASK) == BLACK);
            p_board->hash ^= table[j][i];
        }
    }

    // XOR out the previous castle-rights, enpassantTarget and turn
    p_board->hash ^= p_move->prevEnPassantTarget | p_move->prevCastleRights << 8 | (p_board->turn == WHITE ? BLACK << 10 : WHITE << 10);
    // XOR in the new castle-rights, enpassantTarget and turn
    p_board->hash ^= p_board->enPassantTarget | p_board->castleRights << 8 | p_board->turn << 10;

    return p_board->hash;
}

void m_initZobrist()
{
    initialized = 1;
    //srand(0xdeadbeef);
    for (uint8_t i = 0; i < 64; i++)
    {
        for (uint8_t j = 0; j < 12; j++)
        {
            // Since rand() only returns an integer (usually 32bit)
            // The upper 32 bits of the uint64 also have to be filled
            table[j][i] = rand();
            table[j][i] = (table[j][i] << 32);
            table[j][i] |= rand();
        }
    }
}
