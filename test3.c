#include <stdio.h>
#include "lexer.h"

int main() {
    const char *input = "int x = 5 @ 10;";  // Test case 3
    tokenize(input);
    showTokens();
    return 0;
}