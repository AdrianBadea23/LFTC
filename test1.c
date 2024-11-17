#include <stdio.h>
#include "lexer.h"

int main() {
    const char *input = "3.14.15";  // Test case 1
    tokenize(input);
    showTokens();
    return 0;
}
