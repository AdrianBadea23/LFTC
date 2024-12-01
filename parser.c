#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"
#include "ad.h"

int iTk, blockDepth = 0;   // iterator in tokens
Token *consumed;   // last consumed token

// Domain management
extern Domain *symTable;
extern Symbol *crtFn;

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
bool addNewDomainIfNeeded();
void removeDomainIfNeeded();

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

bool baseType() {
    return consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR);
}

bool isInsideBlock() {
    return blockDepth > 0;
}

bool addNewDomainIfNeeded() {
    if (crtFn != NULL) {
        addDomain();
        
    } else if (isInsideBlock()) {
        
    }
    return true;
}

void removeDomainIfNeeded() {
    
    if (crtFn != NULL) {
        delDomain();
        printf("deletes the current domain\n");
    } else if (isInsideBlock()) {
        delDomain();
        printf("deletes the current domain\n");
    }
}


bool defVar() {
    if (consume(VAR)) {
        if (consume(ID)) {
            const char *name = consumed->text;
            if (consume(COLON)) {
                if (baseType()) {
                    if (searchInCurrentDomain(name)) {
                        tkerr("Variable %s already defined in the current domain", name);
                    }
                    addSymbol(name, KIND_VAR);
                    if (consume(SEMICOLON)) {
                        return true;
                    } else {
                        tkerr("Expected ; after variable declaration");
                    }
                } else {
                    tkerr("Expected base type after variable identifier");
                }
            } else {
                tkerr("Expected : after variable identifier");
            }
        } else {
            tkerr("Expected variable identifier");
        }
    }
    return false;
}


bool funcParam() {
    if (consume(ID)) {
        const char *name = consumed->text;
        if (consume(COLON)) {
            if (baseType()) {
                if (searchInCurrentDomain(name)) {
                    tkerr("Parameter %s already defined in the current domain", name);
                }
				addSymbol(name, KIND_ARG);
                addFnArg(crtFn, name);
                return true;
            } else {
                tkerr("Expected base type for function parameter");
            }
        } else {
            tkerr("Expected : after parameter identifier");
        }
    }
    return false;
}

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

bool defFunc() {
    if (consume(FUNCTION)) {
        if (consume(ID)) {
            const char *name = consumed->text;
            if (searchInCurrentDomain(name)) {
                tkerr("Function %s already defined", name);
            }
            crtFn = addSymbol(name, KIND_FN);
            crtFn->args = NULL;
            addNewDomainIfNeeded();

            if (consume(LPAR)) {
                funcParams(); 
                if (consume(RPAR)) {
                    if (consume(COLON)) {
                        if (baseType()) {
                            while (defVar());  
                            if (block()) {  
                                delDomain();  
                                crtFn = NULL;
                                if (consume(END)) {
                                    return true;
                                } else {
                                    tkerr("Expected END to close the function");
                                }
                            } else {
                                tkerr("Expected function block");
                            }
                        } else {
                            tkerr("Expected base type for function return type");
                        }
                    } else {
                        tkerr("Expected : after function parameter list");
                    }
                } else {
                    tkerr("Expected ) to close function parameter list");
                }
            } else {
                tkerr("Expected ( after function name");
            }
        } else {
            tkerr("Expected function name");
        }
    }
    return false;
}


bool block() {
    if (instr()) {
        while (instr());
        return true;
    }
    return false;
}


bool instr() {
    if (consume(IF)) {
        if (consume(LPAR) && expr() && consume(RPAR)) {
            if (block()) {
                if (consume(ELSE)) {
                    if (!block()) {
                        tkerr("Expected code block after ELSE");
                    }
                }
                if (consume(END)) {
                    return true;
                } else {
                    tkerr("Expected END after IF statement");
                }
            } else {
                tkerr("Expected block after IF condition");
            }
        } else {
            tkerr("Malformed IF statement");
        }
    } else if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            } else {
                tkerr("Expected ; after return expression");
            }
        } else {
            tkerr("Expected expression after RETURN");
        }
    } else if (consume(WHILE)) {
        if (consume(LPAR) && expr() && consume(RPAR)) {
            if (block() && consume(END)) {
                return true;
            } else {
                tkerr("Expected block and END after WHILE");
            }
        } else {
            tkerr("Malformed WHILE statement");
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
                tkerr("Expected expression after AND/OR");
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
        iTk--;
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
            tkerr("Expected expression after comparison operator");
        }
    }
    return true;
}

// exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd() {
    if (!exprMul()) {
        return false;
    }
    while (consume(ADD) || consume(SUB)) {
        if (!exprMul()) {
            tkerr("Expected term after addition/subtraction operator");
        }
    }
    return true;
}

// exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul() {
    if (!exprPrefix()) {
        return false;
    }
    while (consume(MUL) || consume(DIV)) {
        if (!exprPrefix()) {
            tkerr("Expected term after multiplication/division operator");
        }
    }
    return true;
}

// exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
    if (consume(SUB) || consume(NOT)) {
        if (!factor()) {
            tkerr("Expected expression after prefix operator");
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
    if (consume(INT) || consume(REAL) || consume(STR)) {
        return true;
    }
    if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return true;
            } else {
                tkerr("Expected closing ')' in factor");
            }
        } else {
            tkerr("Expected expression in parentheses");
        }
    }
    if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (consume(COMMA)) {
                    if (!expr()) {
                        tkerr("Expected expression after ',' in function call");
                    }
                }
                if (consume(RPAR)) {
                    return true;
                } else {
                    tkerr("Expected closing ')' in function call");
                }
            }
        } else {
            return true;
        }
    }
    return false;
}


bool program() {
    addDomain();
    while (defVar() || defFunc() || block());
    delDomain();
    if (consume(FINISH)) {
        return true;
    } else {
        tkerr("Syntax error");
    }
    return false;
}

void parse() {
    iTk = 0;
    program();
}
