#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"

int iTk;	// the iterator in tokens
Token *consumed;	// the last consumed token

bool funcParams();
bool funcParam();
bool block();
bool instr();
bool expr();
bool exprLogic();
bool exprAssign();
bool exprComp();
bool exprAdd();
bool exprMul();
bool exprPrefix();
bool factor();
bool defVar();
bool program();


// Same as err, but also prints the line of the current token
_Noreturn void tkerr(const char *fmt, ...) {
	fprintf(stderr, "error in line %d: ", tokens[iTk].line);
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

bool consume(int code) {
	if (tokens[iTk].code == code) {
		consumed = &tokens[iTk++];
		return true;
	}
	return false;
}

// baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
bool baseType() {
	return consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR);
}

// defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
bool defFunc() {
	if (consume(FUNCTION)) {
		if (consume(ID)) {
			if (consume(LPAR)) {
				funcParams();
				if (consume(RPAR)) {
					if (consume(COLON)) {
						if (baseType()) {
							while (defVar());
							if (block()) {
								if (consume(END)) {
									return true;
								} else {
									tkerr("Expected end to close up the function");
								}
							} else {
								tkerr("Expected code block after function");
							}
						} else {
							tkerr("Expected function base type");
						}
					} else {
						tkerr("Expected : after ) in function declaration");
					}
				} else {
					tkerr("Expected ) to close the function");
				}
			} else {
				tkerr("Expected ( after function identifier");
			}
		} else {
			tkerr("Expected function identifier");
		}
	}

	return false;
}

// funcParams ::= funcParam ( COMMA funcParam )*
bool funcParams() {
	if (funcParam()) {
		while (consume(COMMA)) {
			if (!funcParam()) {
				tkerr("Error with function parameter after ,");
			}
		}
		return true;
	}
	return false;
}

// funcParam ::= ID COLON baseType
bool funcParam() {
	if (consume(ID)) {
		if (consume(COLON)) {
			if (baseType()) {
				return true;
			} else {
				tkerr("Error with base type of function parameter");
			}
		} else {
			tkerr("Expected : after identifier");
		}
	} else {

	}
	return false;
}

// block ::= instr+
bool block() {
	if (instr()) {
		while (instr());
		return true;
	}
	return false;
}

/*
instr ::= expr? SEMICOLON
		| IF LPAR expr RPAR block ( ELSE block )? END
		| RETURN expr SEMICOLON
		| WHILE LPAR expr RPAR block END
*/

bool instr() {
	if (consume(IF)) {
		if (consume(LPAR) && expr() && consume(RPAR) && block()) {
			if (consume(ELSE)) {
				if (!block()) {
					tkerr("Expected code block after else branch");
				}
			}

			if (consume(END)) {
				return true;
			} else {
				tkerr("Expected end after if statement");
			}
		} else {
			tkerr("Malformed if statement");
		}
	} else if (consume(RETURN)) {
		if (expr()) {
			if (consume(SEMICOLON)) {
				return true;
			} else {
				tkerr("Expected ; after expression");
			}
		} else {
			tkerr("Expected expression after return");
		}
	} else if (consume(WHILE)) {
		if (consume(LPAR) && expr() && consume(RPAR)){
			if(block() && consume(END)) {
				return true;
			} else {
				tkerr("Error in function code block or missing end");
			}
			
		}else{
			tkerr("Error in while statement expression");
		}
	} else if (expr()) {
		if (consume(SEMICOLON)) {
			return true;
		} else {
			tkerr("Expected ; after expression");
		}
	}
	return false;
}

// expr ::= exprLogic
bool expr() {
	return exprLogic();
}

// exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic() {
	if (exprAssign()) {
		while (consume(AND) || consume(OR)) {
			if (!exprAssign()) {
				tkerr("Expected expression after and/or");
			}
		}
		return true;
	}
	return false;
}

// exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign() {
	if (consume(ID)) {
		if (consume(ASSIGN)) {
			return exprComp();
		}
		iTk--; // Backtrack since the assignment was not found
	}
	return exprComp();
}

// exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp() {
	if (!exprAdd()) {
		return false;
	}
	if (consume(LESS) || consume(EQUAL)) {
		if (!exprAdd()) {
			return false;
		}
	}
	return true;
}

// exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd() {
	if (!exprMul()) {
		return false;
	}
	while (consume(ADD)) {
		if (!exprMul()) {
			tkerr("Expected variable when performing addition");
			return false;
		}
	}

	while (consume(SUB)) {
		if (!exprMul()) {
			tkerr("Expected variable when performing subtraction");
			return false;
		}
	}
	return true;
}

// exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul() {
	if (!exprPrefix()) {
		return false;
	}
		while (consume(MUL)) {
		if (!exprMul()) {
			tkerr("Expected variable when performing multiplication");
			return false;
		}
	}

	while (consume(DIV)) {
		if (!exprMul()) {
			tkerr("Expected variable when performing division");
			return false;
		}
	}
	return true;
}

// exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
	if (consume(SUB) || consume(NOT)) {
		if (!factor()) {
			return false;
		}
	} else {
		if (!factor()) {
			return false;
		}
	}
	return true;
}

/*
factor ::= INT
		| REAL
		| STR
		| LPAR expr RPAR
		| ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
*/
bool factor() {
	if (consume(INT)) {
		return true;
	}
	if (consume(REAL)) {
		return true;
	}
	if (consume(STR)) {
		return true;
	}
	if (consume(LPAR)) {
		if (expr()) {
			if (consume(RPAR)) {
				return true;
			} else {
				tkerr("Missing closing ) in factor");
			}
		} else {
			tkerr("Invalid expression inside ()");
		}
	}
	if (consume(ID)) {
		if (consume(LPAR)) {
			if (expr()) {
				while (consume(COMMA)) {
					if (!expr()) {
						tkerr("Invalid expression after ,");
					}
				}
				if (consume(RPAR)) {
					return true;
				} else {
					tkerr("Missing ) for function call");
				}
			} else {
				tkerr("Invalid expression in function call");
			}
		} else {
			return true;
		}
	}
	return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
	if (consume(VAR)) {
		if (consume(ID)) {
			if (consume(COLON)) {
				if (baseType()) {
					if (consume(SEMICOLON)) {
						return true;
					} else {
						tkerr("Expected ; after declaration");
					}
				} else {
					tkerr("Expected base type after identifier");
				}
			} else {
				tkerr("Expected , after variable");
			}
		} else {
			tkerr("Expected variable identifier");
		}
	}

	return false;
}

// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
	for (;;) {
		if (defVar()) {}
		else if (defFunc()) {}
		else if (block()) {}
		else break;
	}
	if (consume(FINISH)) {
		return true;
	} else {
		tkerr("syntax error");
	}
	return false;
}

void parse() {
	iTk = 0;
	program();
}
