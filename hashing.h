#ifndef HASHING_H

#define HASHING_H
#define EVAL_LOOKUP_FAILED -1 // A board evaluation should not be able to have this evaluation

#include <stdint.h>
#include "main.h"
#include "eval.h"
#include "moveHandler.h"

typedef uint64_t hash_t;
typedef enum evaltype_t
{
    EXACT,
    UPPER_BOUND,
    LOWER_BOUND,
} EvalType;

typedef struct hashmap_t
{
    uint64_t numBuckets;
    struct bucket_t **buckets;
} Hashmap;

typedef struct bucket_t
{
    hash_t hash;
    evaluation_t evalScore;
    EvalType evalType;
    uint8_t depth;
    Move move;
    struct bucket_t *p_next;
} Bucket;

void appendToHashmap(Hashmap *p_hashmap, Board *p_board, evaluation_t eval, uint8_t depth, Move move, EvalType evalType);
uint8_t existsInHashmap(Hashmap *p_hashmap, Board *p_board);
evaluation_t getEvaluation(Hashmap *p_hashmap, Board *p_board, uint8_t depth, int64_t alpha, int64_t beta);

Hashmap *createHashmap(uint64_t numBuckets);
void freehashmap(Hashmap *hashmap);

#endif