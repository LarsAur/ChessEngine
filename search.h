#ifndef SEARCH_H

#define SEARCH_H

#include <stdint.h>

#include "main.h"
#include "hashing.h"

Move findBestMove(Board *p_board, uint8_t depth);


#endif