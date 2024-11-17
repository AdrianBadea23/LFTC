#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

#define MAX_INPUT_SIZE 4096

int main(int argc, char* argv[]){
    FILE *fd = fopen(argv[1], "r");

    if(fd == NULL){
        fprintf(stderr, "File not found\n");
        exit(1);
    }

    char input[MAX_INPUT_SIZE];

    size_t bytes_read = fread(input, sizeof(char), MAX_INPUT_SIZE - 1, fd);
    input[bytes_read] = '\0';

    fclose(fd);
    tokenize(input);

    printf("Tokens: \n");
    showTokens();
    
    parse();
    return EXIT_SUCCESS;
}