#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens;

int line = 1, column = 1;  // the current line and column in the input file
int lpara = 0, rpara = 0;

// Adds a token to the end of the tokens list and returns it
// Sets its code and line
Token *addTk(int code) {
    if (nTokens == MAX_TOKENS) err("Too many tokens");
    Token *tk = &tokens[nTokens];
    tk->code = code;
    tk->line = line;
    nTokens++;
    return tk;
}

// Copy in the dst buffer the string between [begin,end)
char *copyn(char *dst, const char *begin, const char *end) {
    char *p = dst;
    if (end - begin > MAX_STR) err("String too long");
    while (begin != end) *p++ = *begin++;
    *p = '\0';
    return dst;
}

void tokenize(const char *pch) {
    const char *start;
    Token *tk;
    char buf[MAX_STR + 1];
    
    while (1) {
        switch (*pch) {
            case ' ': 
            case '\t':
                pch++;
                column++;  // Increment column for whitespace
                break;

            case '\r':
                if (pch[1] == '\n') pch++;  // Skip the newline on Windows
                // Fall through to handle newline

            case '\n':
                line++; 
                pch++; 
                column = 1;  // Reset column on new line
                break;

            case '\0':
                addTk(FINISH); 
                // Check for matching parentheses
                if (lpara != rpara) {
                    err("Invalid number of parentheses.");
                }
                return;

            case ',':
                addTk(COMMA); 
                pch++; 
                column++; 
                break;

            case ':':
                addTk(COLON); 
                pch++; 
                column++; 
                break;

            case ';':
                addTk(SEMICOLON); 
                pch++; 
                column++; 
                break;

            case '(': 
                addTk(LPAR); 
                pch++; 
                lpara++; 
                column++; 
                break;

            case ')': 
                addTk(RPAR); 
                pch++; 
                rpara++; 
                column++; 
                break;

            case '+':
                addTk(ADD); 
                pch++; 
                column++; 
                break;

            case '-':
                addTk(SUB); 
                pch++; 
                column++; 
                break;

            case '*':
                addTk(MUL); 
                pch++; 
                column++; 
                break;

            case '/':
                addTk(DIV); 
                pch++; 
                column++; 
                break;

            case '#':  // Handle comment
                while (*pch != '\n' && *pch != '\0') {
                    pch++; 
                    column++; // Increment column in comments
                }
                break;

            case '=':
                if (pch[1] == '=') {
                    addTk(EQUAL);
                    pch += 2;
                    column += 2; // Update column for both characters
                } else {
                    addTk(ASSIGN);
                    pch++;
                    column++; 
                }
                break;

            case '&':
                if (pch[1] == '&') {
                    addTk(AND);
                    pch += 2;
                    column += 2; // Update column for both characters
                } else {
                    err("Malformed and at line %d, column %d", line, column);
                }
                break;

            case '|':
                if (pch[1] == '|') {
                    addTk(OR);
                    pch += 2;
                    column += 2; // Update column for both characters
                } else {
                    err("Malformed or at line %d, column %d", line, column);
                }
                break;

            case '!':
                if (pch[1] == '=') {
                    addTk(NOTEQ);
                    pch += 2;
                    column += 2; // Update column for both characters
                } else {
                    addTk(NOT);
                    pch++;
                    column++; 
                }
                break;

            case '<':
                if (pch[1] == '=') {
                    addTk(LESSEQ);
                    pch += 2;
                    column += 2; // Update column for both characters
                } else {
                    addTk(LESS);
                    pch++;
                    column++; 
                }
                break;

            case '>':
                if (pch[1] == '=') {
                    addTk(GREATEREQ);
                    pch += 2;
                    column += 2; // Update column for both characters
                } else {
                    addTk(GREATER);
                    pch++;
                    column++; 
                }
                break;

            case '"':
                start = ++pch;  // Skip the opening quote
                int length = 0;
                while (*pch != '"' && *pch != '\0') {
                    if (length >= MAX_STR) {
                        err("String literal too long at line %d, column %d", line, column);
                        return;
                    }
                    pch++;
                    length++;
                    column++; // Increment column for each character in the string
                }
                if (*pch == '"') {
                    tk = addTk(STR);
                    copyn(tk->text, start, pch);
                    pch++;
                    column++; // Increment for the closing quote
                } else {
                    err("Unterminated string at line %d", line);
                }
                break;

            default:
                if (isdigit(*pch)) {
                    start = pch;
                    int number_of_dots = 0;

                    while (isdigit(*pch)) {
                        pch++;
                        column++; // Increment column for each digit
                    }

                    if (*pch == '.') {
                        number_of_dots++;
                        pch++;

                        if (!isdigit(*pch)) {
                            err("Malformed real number at line %d and column %d.", line, column);
                            return;
                        }

                        while (isdigit(*pch)) {
                            pch++;
                            column++; // Increment column for each digit after the dot
                        }

                        // if (!isdigit(*pch)) {
                        //     tokenize(pch);
                        //     err("Malformed real number at line %d and column %d", line, column);
                        //     return;
                        // }

                        tk = addTk(REAL);
                        tk->r = atof(copyn(buf, start, pch));
                    } else {
                        tk = addTk(INT);
                        tk->i = atoi(copyn(buf, start, pch));
                    }
                } else if (isalpha(*pch) || *pch == '_') {
                    start = pch;
                    int id_length = 0;
                    while (isalnum(*pch) || *pch == '_') {
                        if (id_length >= MAX_STR) {
                            err("Identifier too long at line %d, column %d", line, column);
                            return;
                        }
                        pch++;
                        id_length++;
                        column++; // Increment column for each character in identifier
                    }
                    char *text = copyn(buf, start, pch);
                    
                    // Check for keywords
                    if (strcmp(text, "var") == 0) addTk(VAR);
                    else if (strcmp(text, "function") == 0) addTk(FUNCTION);
                    else if (strcmp(text, "if") == 0) addTk(IF);
                    else if (strcmp(text, "else") == 0) addTk(ELSE);
                    else if (strcmp(text, "while") == 0) addTk(WHILE);
                    else if (strcmp(text, "end") == 0) addTk(END);
                    else if (strcmp(text, "return") == 0) addTk(RETURN);
                    else if (strcmp(text, "int") == 0) addTk(TYPE_INT);
                    else if (strcmp(text, "real") == 0) addTk(TYPE_REAL);
                    else if (strcmp(text, "str") == 0) addTk(TYPE_STR);
                    else {
                        tk = addTk(ID);
                        strcpy(tk->text, text);
                    }
                } else {
                    err("Invalid character: %c (%d)", *pch, *pch);
                    return;
                }
                break;
        }
    }
}

void showTokens() {
    printf("[\n");
    for (int i = 0; i < nTokens; i++) {
        Token *tk = &tokens[i];
        char tokenType[MAX_STR];

       switch (tk->code) {
            case FINISH: strcpy(tokenType, "FINISH"); break;
            case COMMA: strcpy(tokenType, "COMMA"); break;
            case COLON: strcpy(tokenType, "COLON"); break;
            case SEMICOLON: strcpy(tokenType, "SEMICOLON"); break;
            case LPAR: strcpy(tokenType, "LPAR"); break;
            case RPAR: strcpy(tokenType, "RPAR"); break;
            case ADD: strcpy(tokenType, "ADD"); break;
            case SUB: strcpy(tokenType, "SUB"); break;
            case AND: strcpy(tokenType, "AND"); break;
            case OR: strcpy(tokenType, "OR"); break;
            case MUL: strcpy(tokenType, "MUL"); break;
            case DIV: strcpy(tokenType, "DIV"); break;
            case EQUAL: strcpy(tokenType, "EQUAL"); break;
            case ASSIGN: strcpy(tokenType, "ASSIGN"); break;
            case NOTEQ: strcpy(tokenType, "NOTEQ"); break;
            case NOT: strcpy(tokenType, "NOT"); break;
            case LESSEQ: strcpy(tokenType, "LESSEQ"); break;
            case LESS: strcpy(tokenType, "LESS"); break;
            case GREATEREQ: strcpy(tokenType, "GREATEREQ"); break;
            case GREATER: strcpy(tokenType, "GREATER"); break;
            case STR: sprintf(tokenType, "STR(\"%s\")", tk->text); break;
            case REAL: sprintf(tokenType, "REAL(%.2f)", tk->r); break;
            case INT: sprintf(tokenType, "INT(%d)", tk->i); break;
            case VAR: strcpy(tokenType, "VAR"); break;
            case FUNCTION: strcpy(tokenType, "FUNCTION"); break;
            case IF: strcpy(tokenType, "IF"); break;
            case ELSE: strcpy(tokenType, "ELSE"); break;
            case WHILE: strcpy(tokenType, "WHILE"); break;
            case END: strcpy(tokenType, "END"); break;
            case RETURN: strcpy(tokenType, "RETURN"); break;
            case TYPE_INT: strcpy(tokenType, "TYPE_INT"); break;
            case TYPE_REAL: strcpy(tokenType, "TYPE_REAL"); break;
            case TYPE_STR: strcpy(tokenType, "TYPE_STR"); break;
            case ID: sprintf(tokenType, "ID(\"%s\")", tk->text); break;
            default: strcpy(tokenType, "UNKNOWN"); break;
        }

        printf("  { \"line\": %d, \"token\": \"%s\" }", tk->line, tokenType);
        if (i < nTokens - 1) printf(",");
        printf("\n");
    }
    printf("]\n");
}
