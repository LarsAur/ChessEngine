#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "book.h"
#include "fen.h"
#include "utils.h"

Move m_getMoveFromUCI(char *uci, Board *p_board)
{
    // Calcultate the rank and file from the uci string
    uint8_t fromFile = uci[0] - 'a';
    uint8_t fromRank = uci[1] - '1';
    uint8_t toFile = uci[2] - 'a';
    uint8_t toRank = uci[3] - '1';

    // Calculate the board index from the rank and file
    uint8_t from = fromFile + fromRank * 8;
    uint8_t to = toFile + toRank * 8;

    // Check for potential promotion
    // The move would be to (2) from (2) and then promotion type (1)
    uint8_t promotion = EMPTY;
    if (strlen(uci) == 5)
    {
        switch (uci[4])
        {
        case 'R':
            promotion = ROOK;
            break;
        case 'N':
            promotion = KNIGHT;
            break;
        case 'B':
            promotion = BISHOP;
            break;
        case 'Q':
            promotion = QUEEN;
            break;
        }
    }

    // Itterate through all legal moves and return the one matching the from, to and promotion
    List *legalMoves = getLegalMoves(p_board);
    Node *p_node = legalMoves->p_head;
    while (p_node)
    {
        Move *p_move = p_node->p_move;
        if (p_move->to == to && p_move->from == from && p_move->promotion == promotion)
        {
            Move move = *p_move;
            freeMoveList(legalMoves);
            return move;
        }

        p_node = p_node->p_next;
    }

    // If there is no matching move, an error has occured
    freeMoveList(legalMoves);
    printf("Error while parsing uci: %ld\n", strlen(uci));

    exit(EXIT_FAILURE);
}

void generateBook(Book *book, uint8_t depth, char *filename)
{
    int line_count = 0;
    size_t line_buf_size = 0;
    char *line_buf = NULL;
    ssize_t line_size = 0;

    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("Unable to open file: %s", filename);
    }

    // Initialize the book without the initialnode and currentnode, they are set at the end because the pointers are moved with realloc
    book->nNodes = 1;
    book->nodes = malloc(sizeof(BookNode));
    book->status = BOOK_READY;

    Board initialBoard;
    createBoardFormFEN(INITIAL_BOARD, &initialBoard);

    book->nodes[0].hash = initialBoard.hash;
    book->nodes[0].nNodes = 0;
    book->nodes[0].moves = NULL;
    book->nodes[0].nodeIndices = NULL;

    // Iterate over all the lines of the file
    // each line represents a game of annotated as pgn
    line_size = getline(&line_buf, &line_buf_size, file);
    while (line_size >= 0)
    {
        line_count++;

        // Create the inital board of the game
        Board board;
        createBoardFormFEN(INITIAL_BOARD, &board);
        // Loop over each uci move in the game and
        char *spaceDelim = " ";
        char *uci = strtok(line_buf, spaceDelim);
        uint64_t currentNodeIndex = 0;
        for (uint8_t i = 0; i < depth; i++)
        {
            Move move = m_getMoveFromUCI(uci, &board);
            performMove(&move, &board);

            // This is the index within the book->nodes list
            uint64_t matchingNodeIndex = 0;
            // Check if the new board exists in the book
            uint8_t board_exists = 0;
            for (uint64_t i = 0; i < book->nNodes; i++)
            {
                if (board.hash == book->nodes[i].hash)
                {
                    board_exists = 1;
                    matchingNodeIndex = i;
                    break;
                }
            }

            if (!board_exists)
            {
                // Create a new node and add it to the book nodes
                book->nodes = realloc(book->nodes, ++book->nNodes * sizeof(BookNode));

                // Set the matching-node-index to the index of the new node, which is the last
                matchingNodeIndex = book->nNodes - 1;
                book->nodes[matchingNodeIndex].hash = board.hash;
                book->nodes[matchingNodeIndex].nNodes = 0;
                book->nodes[matchingNodeIndex].moves = NULL;
                book->nodes[matchingNodeIndex].nodeIndices = NULL;

                // Add the new possition into the current node
                BookNode *currentNode = &book->nodes[currentNodeIndex];
                currentNode->nodeIndices = realloc(currentNode->nodeIndices, ++currentNode->nNodes * sizeof(uint64_t));
                currentNode->nodeIndices[currentNode->nNodes - 1] = matchingNodeIndex;
                currentNode->moves = realloc(currentNode->moves, currentNode->nNodes * sizeof(Move));
                currentNode->moves[currentNode->nNodes - 1] = move;
            }
            else
            {
                // If the new board exists in the book, check if it exists in the position of the current node
                uint8_t board_exists = 0;
                for (uint8_t i = 0; i < book->nodes[currentNodeIndex].nNodes; i++)
                {
                    if (board.hash == book->nodes[book->nodes[currentNodeIndex].nodeIndices[i]].hash)
                    {
                        board_exists = 1;
                        break;
                    }
                }

                // if the board does not exist in the current node, add it
                if (!board_exists)
                {
                    BookNode *currentNode = &book->nodes[currentNodeIndex];
                    currentNode->nodeIndices = realloc(currentNode->nodeIndices, ++currentNode->nNodes * sizeof(uint64_t));
                    currentNode->nodeIndices[currentNode->nNodes - 1] = matchingNodeIndex;
                    currentNode->moves = realloc(currentNode->moves, currentNode->nNodes * sizeof(Move));
                    currentNode->moves[currentNode->nNodes - 1] = move;
                }
            }

            // Set the new current node
            currentNodeIndex = matchingNodeIndex;

            uci = strtok(NULL, spaceDelim);
            // check if it contains the final score eg. "1/2-1/2" or "1-0"
            if(uci == NULL || uci[0] == '1' || uci[0] == '0')
            {
                break;
            }
        }

        line_size = getline(&line_buf, &line_buf_size, file);
    }

    book->initialNode = &book->nodes[0];
    book->currentNode = &book->nodes[0];

    free(line_buf);
    line_buf = NULL;
    fclose(file);
}

// Advances the book with the move, by changing the current node, and sets the book status
void advanceInBook(Book *book, Move move)
{
    if (book->status == BOOK_ENDED)
        return;

    // Loop through all the possible moves and check if it matches any of the possible moves
    for (uint8_t i = 0; i < book->currentNode->nNodes; i++)
    {
    
        printMove(&book->currentNode->moves[i]);
        
        if(move.from == book->currentNode->moves[i].from
            && move.to == book->currentNode->moves[i].to
            && move.promotion == book->currentNode->moves[i].promotion
        )
        {
            uint64_t nextNodeIndex = book->currentNode->nodeIndices[i];
            book->currentNode = &book->nodes[nextNodeIndex];

            if(book->currentNode->nNodes == 0)
            {
                book->status = BOOK_ENDED;
            }

            return;
        }
    }

    // If the moves that was played is not in the book, the book is out of the opening
    book->status = BOOK_ENDED;
}

// Selects a move from the move list, the current node is also changed and status of the board is set
void getNextMove(Book *book, Move *move)
{
    // If there are no moves in the current position, the book has ended
    if (book->status == BOOK_ENDED)
        return;

    srand(time(NULL));
    uint8_t randIndex = rand() % book->currentNode->nNodes;
    *move = book->currentNode->moves[randIndex];

    printf("Rand Index: %d\n", randIndex);

    // Advance the to the next node
    uint64_t nextNodeIndex = book->currentNode->nodeIndices[randIndex];
    book->currentNode = &book->nodes[nextNodeIndex];

    if (book->currentNode->nNodes == 0)
    {
        book->status = BOOK_ENDED;
    }
}

void freeBook(Book *book)
{
    for (uint64_t i = 0; i < book->nNodes; i++)
    {
        free(book->nodes[i].moves);
        free(book->nodes[i].nodeIndices);
    }

    free(book->nodes);
}