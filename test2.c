#include <stdio.h>
#include "lexer.h"

int main() {
    const char *input = "\"This is a string";  // Test case 2
    tokenize(input);
    showTokens();
    return 0;
}