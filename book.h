#ifndef OPENINGBOOK_H

#define OPENINGBOOK_H
#define BOOK_ENDED 1
#define BOOK_READY 0

#include "hashing.h"
#include "moveHandler.h"

typedef struct book_node_t
{
    hash_t hash;            // Hash of the board represented by node, this is utilzied when building the book
    Move *moves;            // List of all possible moves
    uint64_t *nodeIndices;  // List of all node indices corresponding the moves 
    uint8_t nNodes;         // Number of moves in the position
} BookNode;

typedef struct book_t
{
    struct book_node_t *currentNode;    // The current node of the board
    struct book_node_t *initialNode;    // The first node in the book, initial board
    struct book_node_t *nodes;          // List of all nodes in the book
    uint64_t nNodes;                         // Number of nodes (boards) in the book
    int status;                         // BOOK_ENDED, or BOOK_READY
} Book;

void generateBook(Book *book, uint8_t depth, char* filename);
void advanceInBook(Book *book, Move move); // Advances the book with the move, by changing the current node, and returns the book status
void getNextMove(Book *book, Move *move); // Selects a move from the move list, the current node is also changed and status is returned
void freeBook(Book *book);

#endif