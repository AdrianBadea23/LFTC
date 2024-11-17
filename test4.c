#include <stdio.h>
#include "lexer.h"

int main() {
    const char *input = "(x + (y * 2";  // Test case 4
    tokenize(input);
    showTokens();
    return 0;
}