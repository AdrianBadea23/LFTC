#include <stdio.h>
#include "lexer.h"

int main() {
    const char *input = "var x = 5a;";  // Test case 6
    tokenize(input);
    showTokens();
    return 0;
}