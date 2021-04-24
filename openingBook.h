#ifndef OPENINGBOOK_H

#define OPENINGBOOK_H
#define BOOK_ENDED 0
#define BOOK_ADVANCED 1

#include "hashing.h"
#include "moveHandler.h"

typedef struct book_node_t
{
    hash_t hash;        // Hash of the board represented by node, this is utilzied when building the book
    uint8_t nMoves;     // Number of moves in the position
    Move **moves;       // List of all possible moves
    Move **book_node_t; // List of all nodes corresponding the moves 
} BookNode;

typedef struct book_t
{
    struct book_node_t currentNode;
    struct book_node_t initialNode;
} Book;

void generateBook(Book *book, uint8_t depth, char* filename);
uint8_t advanceInBook(Book *book, Move move); // Advances the book with the move, by changing the current node, and returns the book status
uint8_t getNextMove(Book *book, Move *move); // Selects a move from the move list, the current node is also changed and status is returned

#endif