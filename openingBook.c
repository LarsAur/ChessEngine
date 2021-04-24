#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openingBook.h"

void generateBook(Book *book, uint8_t depth, char *filename)
{
    int nNodes = 1;
    BookNode *nodes = malloc(nNodes * sizeof(BookNode));

    FILE *file = fopen(filename, "r");

    if(!file)
    {
        printf("Unable to open file: %s", filename);
    }


    // Find the size of the file
    fseek(file, 0, SEEK_END);
    ssize_t file_size = ftell(file);
    printf("Reading file of size %ld Bytes\n", file_size);

    char buffer[file_size + 1];
    ssize_t size_read = fread(buffer, file_size, file_size, file);
    buffer[file_size - 1] = '\0';

    char delim[2] = "\n";
    char *line = strtok(buffer, delim);
    while(line)
    {
        printf("%s", line);
        line = strtok(NULL, delim);
    }

    fclose(file);
}

// Advances the book with the move, by changing the current node, and returns the book status
uint8_t advanceInBook(Book *book, Move move)
{
}

// Selects a move from the move list, the current node is also changed and status is returned
uint8_t getNextMove(Book *book, Move *move)
{
}
