#include <stdio.h>
#include "lexer.h"

int main() {
    const char *input = "asdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdasasdasdasdasdasdas";  // Test case 7
    tokenize(input);
    showTokens();
    return 0;
}