#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "main.h"
#include "moveHandler.h"
#include "utils.h"
#include "fen.h"
#include "hashing.h"

void m_testNumberOfPseudoLegalMoves(char *FEN, uint16_t expected, char *msg);
void m_recursiveMove(uint8_t depth, uint8_t height, Board *p_board, Move *p_prevMove, uint64_t *p_counter, uint64_t *p_epCounter, uint64_t *p_castleCounter, uint64_t *p_checkMateCounter);

void __test__FEN()
{
    Board board;
    createBoardFormFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", &board);
    if(board.castleRights != 0b1111)
    {
        printf("FEN casteling error 1");
    }

    createBoardFormFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w - - 0 1", &board);
    if(board.castleRights != 0b0000)
    {
        printf("FEN casteling error 2");
    }
}

void __test__pawnMovement()
{
    // White moves
    m_testNumberOfPseudoLegalMoves("8/pppppppp/8/8/8/8/PPPPPPPP/8 w - - 0 1", 16, "Wrong number of white pawn moves from initial position");

    // Black moves
    m_testNumberOfPseudoLegalMoves("8/pppppppp/8/8/8/8/PPPPPPPP/8 b - - 0 1", 16, "Wrong number of black pawn moves from initial position");

    // Pawn promotion
    m_testNumberOfPseudoLegalMoves("8/PPPPPPPP/8/8/8/8/8/8 w - - 0 1", 32, "Wrong number of promotion moves");

    // White pawn takes
    m_testNumberOfPseudoLegalMoves("8/pppppppp/PPPPPPPP/8/8/8/8/8 w - - 0 1", 14, "Wrong number of pawn takes for white");

    // Black pawn takes
    m_testNumberOfPseudoLegalMoves("8/pppppppp/PPPPPPPP/8/8/8/8/8 b - - 0 1", 14, "Wrong number of pawn takes for black");

    // White pawn takes into promotion
    m_testNumberOfPseudoLegalMoves("rnbqkbnr/PPPPPPPP/8/8/8/8/8/8 w - - 0 1", 56, "Wrong number of pawn takes into promotion for white");

    // Black pawn takes into promotion
    m_testNumberOfPseudoLegalMoves("8/8/8/8/8/8/pppppppp/RNBQKBNR b - - 0 1", 56, "Wrong number of pawn takes into promotion for black");
}

void __test__slidingMovement()
{
    // White rook move on empty board
    m_testNumberOfPseudoLegalMoves("8/8/8/4R3/8/8/8/8 w - - 0 1", 14, "Wrong number of white rook moves on empty board");

    // White rook stop after takes
    m_testNumberOfPseudoLegalMoves("8/8/4r3/3rRr2/4r3/8/8/8 w - - 0 1", 4, "Wrong number of white rook moves when only takes are available");

    // White rook locked by its own pieces
    m_testNumberOfPseudoLegalMoves("RP6/P7/8/8/8/8/8/8 w - - 0 1", 0, "Wrong number of white rook moves when locked in by own pieces");

    // White bishop move on empty board
    m_testNumberOfPseudoLegalMoves("8/8/8/4B3/8/8/8/8 w - - 0 1", 13, "Wrong number of white bishop moves on empty board");

    // White bishop stop after takes
    m_testNumberOfPseudoLegalMoves("8/8/3b1b2/4B3/3b1b2/8/8/8 w - - 0 1", 4, "Wrong number of white bishop moves when only takes are available");

    // White bishop locked by its own pieces
    m_testNumberOfPseudoLegalMoves("BP6/1P6/8/8/8/8/8/8 w - - 0 1", 0, "Wrong number of white bishop moves when locked in by own pieces");

    // White queen move on empty board
    m_testNumberOfPseudoLegalMoves("8/8/8/4Q3/8/8/8/8 w - - 0 1", 27, "Wrong number of white queen moves on empty board");

    // White queen stop after takes
    m_testNumberOfPseudoLegalMoves("8/8/3bbb2/3bQb2/3bbb2/8/8/8 w - - 0 1", 8, "Wrong number of white queen moves when only takes are available");

    // White queen locked by its own pieces
    m_testNumberOfPseudoLegalMoves("QP6/PP6/8/8/8/8/8/8 w - - 0 1", 0, "Wrong number of white queen moves when locked in by own pieces");
}

void __test__kingMovement()
{
}

void __test__knightMovement()
{
    m_testNumberOfPseudoLegalMoves("N7/8/8/8/8/8/8/8 w - - 0 1", 2, "Wrong number of knight moves from corner");
    
    m_testNumberOfPseudoLegalMoves("8/3p1p2/2p3p1/4N3/2p3p1/3p1p2/8/8 w - - 0 1", 8, "Wrong number of knight moves to target 8 pieces");
    
    m_testNumberOfPseudoLegalMoves("8/8/3P1P2/2P3P1/4N3/2P3P1/3P1P2/8 w - - 0 1", 10, "Wrong number of knight moves when locked by own pieces");
}

void __test__moveTree()
{
    uint8_t const DEPTH = 5;
    uint64_t posCount = 0;
    uint64_t epCount = 0;
    uint64_t castleCount = 0;
    uint64_t checkmates = 0;

    // Benchmark: DEPTH = 6 board = INITIAL_BOARD time = 183.8 sec

    Board board;
    createBoardFormFEN(INITIAL_BOARD, &board);
    
    double timeSpent = 0.0;
    clock_t begin = clock();
    m_recursiveMove(DEPTH, DEPTH, &board, NULL, &posCount, &epCount, &castleCount, &checkmates);
    clock_t end = clock();

    // calculate elapsed time by finding difference (end - begin) and
    // dividing the difference by CLOCKS_PER_SEC to convert to seconds
    timeSpent += (double)(end - begin) / CLOCKS_PER_SEC;
 
    printf("Time elapsed is %f seconds\n", timeSpent);

    printf("Number of positions in tree with depth %d: %ld\n", DEPTH, posCount);
    printf("En Passants: %ld\n", epCount);
    printf("Castles: %ld\n", castleCount);
    printf("Check mates: %ld\n", checkmates);
}

void __test__hashmap()
{
    Hashmap *p_hashmap = createHashmap(1024);

    Board board;
    Move move;
    createBoardFormFEN(INITIAL_BOARD, &board);

    appendToHashmap(p_hashmap, &board, evaluateBoard(&board, 0), 0, move, EXACT);
    
    if(!existsInHashmap(p_hashmap, &board))
    {
        printf("The board did not exist in the hashmap after being appended");
    }

    if(getEvaluation(p_hashmap, &board, 0, 0, 0) != evaluateBoard(&board, 0))
    {
        printf("The evaluation of the board in the hashmap was wrong");
    }
}

void m_recursiveMove(uint8_t depth, uint8_t height, Board *p_board, Move *p_prevMove, uint64_t *p_counter, uint64_t *p_epCounter, uint64_t *p_castleCounter, uint64_t *p_checkMateCounter){
    if(depth == 0){
        (*p_counter)++;

        if(p_prevMove->enPassant)
        {
            (*p_epCounter)++;
        }
        
        if(p_prevMove->castle)
        {
            (*p_castleCounter)++;
        }

        List *p_legalMoves = getLegalMoves(p_board);

        if(p_legalMoves->length == 0)
        {
            (*p_checkMateCounter)++;
        }

        freeMoveList(p_legalMoves);
        
        //printBoard(p_board);
        //printf("----------------\n");

        return;
    }

    List *p_legalMoves = getLegalMoves(p_board);
    Node *p_node = p_legalMoves->p_head;

    if(p_legalMoves->length == 0)
    {
        (*p_checkMateCounter)++;
    }

    for(uint8_t i = 0; i < p_legalMoves->length; i++)
    {  
        performMove(p_node->p_move, p_board);

        if(depth == height)
        {
            uint64_t count = 0;
            m_recursiveMove(depth - 1, height, p_board, p_node->p_move, &count, p_epCounter, p_castleCounter, p_checkMateCounter);
            (*p_counter) += count;
            printf("Count %ld after ", count);
            printMove(p_node->p_move);
        }
        else
        {
            m_recursiveMove(depth - 1, height, p_board, p_node->p_move, p_counter, p_epCounter, p_castleCounter, p_checkMateCounter);
        }
        undoMove(p_node->p_move, p_board);

        p_node = p_node->p_next;
    }

    freeMoveList(p_legalMoves);
}   

// Tests the board from a given FEN and checks if the correct number of pseudolegal moves are found
void m_testNumberOfPseudoLegalMoves(char *FEN, uint16_t expected, char *msg)
{
    Board *p_board = malloc(sizeof(Board));
    createBoardFormFEN(FEN, p_board);

    List *p_pseudoMoves = getPseudoLegalMoves(p_board);
    if (p_pseudoMoves->length != expected)
    {
        printf("%s, Got %d, expected %d\n", msg, p_pseudoMoves->length, expected);
        printBoard(p_board);
        printMoveList(p_pseudoMoves);
    }

    freeMoveList(p_pseudoMoves);
    free(p_board);
}