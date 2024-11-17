#pragma once

enum {
    ID,
    TYPE_INT, TYPE_REAL, TYPE_STR,
    VAR, FUNCTION, IF, ELSE, WHILE, END, RETURN,
    COMMA, COLON, SEMICOLON, LPAR, RPAR, FINISH,
    ADD, SUB, MUL, DIV, AND, OR, NOT, ASSIGN, EQUAL, NOTEQ, LESS, GREATER, GREATEREQ, LESSEQ,
    INT, REAL, STR,
};


#define MAX_STR		127

typedef struct{
	int code;		// ID, TYPE_INT, ...
	int line;		// the line from the input file
	union{
		char text[MAX_STR+1];		// the chars for ID, STR
		int i;		// the value for INT
		double r;		// the value for REAL
		};
	}Token;

#define MAX_TOKENS		4096
extern Token tokens[];
extern int nTokens;

void tokenize(const char *pch);
void showTokens();

