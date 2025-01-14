#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"
#include "ad.h"
#include "ad.h"
#include "gen.h"

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

bool baseType(void) {
    // return consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR);

    if (consume(TYPE_INT)) {
        ret.type = TYPE_INT;
        return true;
    }
    if (consume(TYPE_REAL)) {
        ret.type = TYPE_REAL;
        return true;
    }
    if (consume(TYPE_STR)) {
        ret.type = TYPE_STR;
        return true;
    }


    return false;
}

bool defVar(void) {
    int start = iTk;

    if (consume(VAR)) {
        if (consume(ID)) {
            const char *name = consumed->text;
            Symbol *s = searchInCurrentDomain(name);
            if (s)
                tkerr("Symbol redefinition: %s\n", name);
            s = addSymbol(name, KIND_VAR);
            s->local = crtFn != NULL;

            if (consume(COLON)) {
                s->type = ret.type;

                if (baseType()) {
                    if (consume(SEMICOLON)) {
                        Text_write(crtVar, "%s %s;\n", cType(s->type), s->name);

                        return true;
                    } tkerr("Expected ';' after variable declaration");
                } tkerr("Expected base type after ':' in variable declaration");
            } tkerr("Expected ':' after variable name");
        } tkerr("Expected variable name after 'VAR'");
    }

    iTk = start;
    return false;
}

bool factor(void) {
    if (consume(INT)) {
        Text_write(crtCode, "%d", consumed->i);
        setRet(TYPE_INT, false); // INT is not an l-value
        return true;
    }

    if (consume(REAL)) {
        Text_write(crtCode, "%g", consumed->r);
        setRet(TYPE_REAL, false); // REAL is not an l-value
        return true;
    }

    if (consume(STR)) {
        Text_write(crtCode,"\"%s\"",consumed->text);
        setRet(TYPE_STR, false); // STR is not an l-value
        return true;
    }

    if (consume(LPAR)) {
        Text_write(crtCode, "(");
        if (expr()) {
            if (consume(RPAR)) {
                Text_write(crtCode, ")");
                return true; // Successfully parsed ( expr )
            } tkerr("Expected closing parenthesis");
        } tkerr("Invalid expression inside parentheses");
    }

    if (consume(ID)) {
        const Symbol *s = searchSymbol(consumed->text);
        if (!s) {
            tkerr("Undefined symbol: %s", consumed->text);
        }

        Text_write(crtCode, "%s", s->name);

        if (consume(LPAR)) {
            // Function call logic
            Text_write(crtCode, "(");
            if (s->kind != KIND_FN) {
                tkerr("Symbol is not a function: %s", consumed->text);
            }

            const Symbol *arg = s->args;
            if (consume(RPAR)) {
                if (arg) {
                    tkerr("Too few arguments in function call: %s", s->name);
                }
                Text_write(crtCode, ")");
                setRet(s->type, false); // Function call returns its type
                return true;
            }

            bool firstArgument = true;
            do {

                if (!firstArgument) {
                    Text_write(crtCode, ",");
                }
                firstArgument = false;

                if (!expr()) {
                    tkerr("Invalid argument in function call");
                }

                if (!arg) {
                    tkerr("Too many arguments in function call: %s", s->name);
                }
                if (arg->type != ret.type) {
                    tkerr("Argument type mismatch in function call: %s", s->name);
                }

                arg = arg->next;
            } while (consume(COMMA));

            if (consume(RPAR)) {
                if (arg) {
                    tkerr("Too few arguments in function call: %s", s->name);
                }
                Text_write(crtCode, ")");
                setRet(s->type, false); // Function call returns its type
                return true;
            }

            tkerr("Expected closing parenthesis after function call");
        }


        // Handle cases where ID is not followed by LPAR
        if (s->kind == KIND_FN) {
            tkerr("The function %s can only be called", s->name);
        }
        setRet(s->type, true); // Variables can be l-values
        return true;
    }


    return false; // No valid `factor` matched
}


// exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
    if (consume(SUB)) {
        Text_write(crtCode, "-");

        if (!factor()) {
            tkerr("Expected expression after unary operator");
        }

        if (ret.type == TYPE_STR) {
            tkerr("The operand of a unary operator must NOT be a string");
        }

        ret.lval = false;
        return true;
    }

    if (consume(NOT)) {
        Text_write(crtCode, "!");

        if (!factor()) {
            tkerr("Expected expression after unary operator");
        }

        if (ret.type == TYPE_STR) {
            tkerr("The operand of a unary operator must NOT be a string");
        }

        setRet(TYPE_INT, false);
        return true;
    }

    return factor();
}


bool exprMul() {
    if (exprPrefix()) {
        while (consume(MUL) || consume(DIV)) {
            Ret leftType = ret;

            if (leftType.type == TYPE_STR) {
                tkerr("The left operand of a * or / must NOT be a string");
            }

            if (consume(MUL)) {
                Text_write(crtCode, "*");
            } else if (consume(DIV)) {
                Text_write(crtCode, "/");
            }

            if (!exprPrefix()) {
                tkerr("Invalid expression after * or /");
            }

            if (leftType.type != ret.type) {
                tkerr("Type mismatch in multiplication or division");
            }

            ret.lval = false;
        }
        return true;
    }
    return false;
}

bool exprAdd() {
    if (exprMul()) {
        while (true) {
            Ret leftType = ret;

            if (leftType.type == TYPE_STR) {
                // tkerr("The left operand of a + or - must NOT be a string");
            }

            // Check which operator is consumed
            if (consume(ADD)) {
                Text_write(crtCode, "+");
            } else if (consume(SUB)) {
                Text_write(crtCode, "-");
            } else {
                break; // Exit loop if no operator is consumed
            }

            if (!exprMul()) {
                tkerr("Expected expression after '+' or '-'");
            }

            if (leftType.type != ret.type) {
                tkerr("Type mismatch in addition or subtraction");
            }

            ret.lval = false;
        }
        return true;
    }
    return false;
}


bool exprComp(void) {
    // Parse the first operand
    if (exprAdd()) {
        Ret leftType = ret; // Store the type of the left operand

        if (consume(LESS)) {
            Text_write(crtCode, "<");
        } else if (consume(GREATER)) {
            Text_write(crtCode, ">");
        } else if (consume(EQUAL)) {
            Text_write(crtCode, "==");
        } else if (consume(LESSEQ)) {
            Text_write(crtCode, "<=");
        } else if (consume(GREATEREQ)) {
            Text_write(crtCode, ">=");
        } else if (consume(NOTEQ)) {
            Text_write(crtCode, "!=");
        } else {
            return true; // No operator means this is just an exprAdd
        }

        // Parse the second operand
        if (!exprAdd()) {
            tkerr("Expected expression after comparison operator");
        }

        // Type check
        if (leftType.type != ret.type) {
            tkerr("Type mismatch in comparison");
        }

        setRet(TYPE_INT, false); // Comparison returns TYPE_INT
        return true; // Successfully parsed a comparison expression
    }
    return false; // Failed to parse a comparison expression
}

bool exprAssign() {
    int start = iTk;
    if (consume(ID)) {
        const char *name = consumed->text;

        if (consume(ASSIGN)) {
            Text_write(crtCode, "%s=", name);

            if (exprComp()) {
                Symbol *s = searchSymbol(name);
                if (!s)
                    tkerr("Undefined symbol: %s\n", name);
                if (s->kind == KIND_FN)
                    tkerr("Cannot assign to function: %s\n", name);
                if (s->type != ret.type)
                    tkerr("Type mismatch in assignment to symbol: %s\n", name);
                ret.lval = false;

                return true;
            }
            tkerr("Expected expression after '='");
        }
    }

    iTk = start;
    return exprComp();
}

// exprCompLogical ::= exprAdd ( ( LESS | EQUAL | GREATER | LESS_EQUAL | GREATER_EQUAL | NOT_EQUAL ) exprAdd )?
bool exprCompLogical(void) {
    if (exprAdd()) {
        // Check for all comparison operators
        if (consume(LESS) || consume(EQUAL) || consume(GREATEREQ) ||
            consume(LESSEQ) || consume(GREATEREQ) || consume(NOTEQ) || consume(LESSEQ) || consume(NOTEQ)) {
            if (!exprAdd()) {
                tkerr("Expected expression after comparison operator");
            }
            Text_write(crtCode, " %s ", consumed->text);
        }

        printf("Current token: %d\n", tokens[iTk].code);

        return true;
    }

    return false;
}

// exprLogical ::= exprCompLogical ( ( AND | OR ) exprCompLogical )*
bool exprLogical() {
    // Parse the first operand
    if (exprCompLogical()) {
        // Continue parsing logical operators and their right-hand operands
        while (consume(AND) || consume(OR)) {
            // Store the type of the left operand
            Ret leftType = ret;

            // Ensure the left operand is not a string
            if (leftType.type == TYPE_STR) {
                tkerr("The left operand of a logical operator must NOT be a string");
            }

            // Parse the right-hand operand
            if (!exprCompLogical()) {
                tkerr("Expected expression after logical operator");
            }

            // Ensure the right operand is not a string
            if (ret.type == TYPE_STR) {
                tkerr("The right operand of a logical operator must NOT be a string");
            }

            // Logical operations result in an integer type
            setRet(TYPE_INT, false);
        }
        return true; // Successfully parsed a logical expression
    }
    return false; // Failed to parse a logical expression
}

bool expr(void) {
    return exprAssign();
}

// instr ::= expr? SEMICOLON | IF LPAR expr RPAR block ( ELSE block )? END | RETURN expr SEMICOLON | WHILE LPAR expr RPAR block END
bool instr(void) {
    if (consume(SEMICOLON)) {
        Text_write(crtCode, ";\n");
        return true;
    }

    if (expr()) {
        if (consume(SEMICOLON)) {
            Text_write(crtCode, ";\n");
            return true;
        }
        tkerr("Missing semicolon after instr");
    }

    if (consume(IF)) {
        if (consume(LPAR)) {
            Text_write(crtCode, "if(");

            if (expr()) {
                if (!crtFn)
                    tkerr("IF statement outside function");
                if (ret.type != crtFn->type)
                    tkerr("IF statement type mismatch");

                if (consume(RPAR)) {
                    Text_write(crtCode, "){\n");

                    if (block()) {
                        Text_write(crtCode, "}\n");
                        if (consume(ELSE)) {
                            Text_write(crtCode, "else{\n");
                            if (!block()) {
                                return false;
                            }
                            Text_write(crtCode, "}\n");
                        }
                        if (consume(END)) {
                            return true;
                        } else {
                            tkerr("Missing END in IF statement");
                        }
                    } else {
                        tkerr("Expected block after IF condition");
                    }
                } else {
                    tkerr("Expected ')' after IF condition");
                }
            } else {
                tkerr("Expected expression in IF condition");
            }
        } else {
            tkerr("Expected '(' after IF");
        }
    }

    if (consume(RETURN)) {
        Text_write(crtCode, "return ");
        if (expr()) {
            if (consume(SEMICOLON)) {
                Text_write(crtCode, ";\n");

                return true;
            } tkerr("RETURN statement missing semicolon");
        } tkerr("RETURN statement missing expression");
    }

    if (consume(WHILE)) {
        Text_write(crtCode, "while(");

        if (consume(LPAR)) {
            if (expr()) {
                if (ret.type == TYPE_STR)
                    tkerr("the while condition must NOT be a string");

                if (consume(RPAR)) {
                    Text_write(crtCode, "){\n");

                    if (block()) {
                        if (consume(END)) {
                            Text_write(crtCode, "}\n");
                            return true;
                        } else {
                            tkerr("Missing END in WHILE loop");
                        }
                    } else {
                        tkerr("Expected block after WHILE condition");
                    }
                } else {
                    tkerr("Expected ')' after WHILE condition");
                }
            } else {
                tkerr("Expected expression in WHILE condition");
            }
        } else {
            tkerr("Expected '(' after WHILE");
        }
    }

    return false;
}

// block ::= instr+
bool block(void) {
    if (instr()) {
        while (instr()) {}

        return true;
    }

    return false;
}

// funcParam ::= ID COLON baseType
bool funcParam(void) {
    if (consume(ID)) {
        const char *name = consumed->text;
        Symbol *s = searchInCurrentDomain(name);
        if (s)
            tkerr("Symbol redefinition: %s\n", name);
        s = addSymbol(name, KIND_ARG);
        Symbol *sFnParam = addFnArg(crtFn, name);

        if (consume(COLON)) {
            if (baseType()) {
                Text_write(&tFnHeader, "%s %s", cType(ret.type), name);

                s->type = ret.type;
                sFnParam->type = ret.type;

                return true;
            } tkerr("Expected base type after ':' in parameter definition");
        } tkerr("Expected ':' after parameter name");
    }
    return false;
}

bool funcParams(void) {
    if (funcParam()) {
        while (1) {
            if (consume(COMMA)) {
                Text_write(&tFnHeader, ",");
                if (!funcParam()) {
                    tkerr("Expected parameter after comma");
                }
            } else {
                break;
            }
        }
        return true;
    }

    return true;
}

bool defFunc(void) {
    const int start = iTk;

    if (consume(FUNCTION)) {
        if (consume(ID)) {
            const char *name = consumed->text;

            crtCode = &tFunctions;
            crtVar = &tFunctions;
            Text_clear(&tFnHeader);
            Text_write(&tFnHeader, "%s(", name);

            const Symbol *s = searchInCurrentDomain(name);
            if (s)
                tkerr("Symbol redefinition: %s\n", name);
            crtFn = addSymbol(name, KIND_FN);
            crtFn->args = NULL;
            addDomain();

            if (consume(LPAR)) {
                if (funcParams()) {
                    if (!consume(RPAR)) {
                        // Ensure we have a closing parenthesis
                        tkerr("Expected ')' after function parameters");
                    }
                }

                if (consume(COLON)) {
                    // Check for the colon after the parameters
                    if (baseType()) {
                        Text_write(&tFunctions, "\n%s %s){\n", cType(ret.type), tFnHeader.buf);

                        crtFn->type = ret.type;

                        // Ensure there is a valid return type
                        // Parse variable definitions and the block body
                        while (defVar()) {
                        }
                        if (block()) {
                            if (consume(END)) {
                                Text_write(&tFunctions, "}\n");
                                crtCode = &tMain;
                                crtVar = &tBegin;

                                delDomain();
                                crtFn = NULL;

                                // Ensure we have the END keyword
                                return true;
                            } tkerr("Expected 'END' after function body");
                        }
                    } else {
                        tkerr("Expected return type after ':' in function definition");
                    }
                } else {
                    tkerr("Expected ':' after function parameter list");
                }
            }
            tkerr("Expected '(' after function name");
        }
        tkerr("Expected function name after 'FUNCTION'");
    }

    iTk = start;
    return false;
}


// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
    addDomain();

    addPredefinedFns();

    crtCode = &tMain;
    crtVar = &tBegin;
    Text_write(&tBegin, "#include \"quick.h\"\n\n");
    Text_write(&tMain, "\nint main(){\n");

    while (defVar() || defFunc() || block()) {
    }
    if (consume(FINISH)) {
        delDomain();

        Text_write(&tMain,"return 0;\n}\n");
        FILE *fis=fopen("./test/1.c","w");
        if(!fis){
            printf("cannot write to file 1.c\n");
            exit(EXIT_FAILURE);
        }
        fwrite(tBegin.buf,sizeof(char),tBegin.n,fis);
        fwrite(tFunctions.buf,sizeof(char),tFunctions.n,fis);
        fwrite(tMain.buf,sizeof(char),tMain.n,fis);
        fclose(fis);

        printf("Generated code\n");

        return true;
    }

    tkerr("Missing FINISH at end of program");
}

void parse() {
    iTk = 0;
    program();
}
