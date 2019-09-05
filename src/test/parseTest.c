#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test/parseTest.h"
#include "main/parse.h"
#include "main/util.h"

int tokensEqualVoid(void* a, void* b);

/**
 * Determines if two tokens are equal.
 */
int tokensEqual(Token* a, Token* b) {
    if(a == NULL || b == NULL || a->type != b->type) {
        return 0;
    }
    if(a->type == TOKEN_INT) {
        return ((IntToken*) a)->value == ((IntToken*) b)->value;
    } else if(a->type == TOKEN_LITERAL) {
        return strcmp(((LiteralToken*) a)->value, ((LiteralToken*) b)->value) == 0;
    } else if(a->type == TOKEN_IDENTIFIER) {
        return strcmp(((IdentifierToken*) a)->value, ((IdentifierToken*) b)->value) == 0;
    } else if(a->type == TOKEN_BINARY_OP) {
        BinaryOpToken* aBinOp = (BinaryOpToken*) a;
        BinaryOpToken* bBinOp = (BinaryOpToken*) b;
        return strcmp(aBinOp->op, bBinOp->op) == 0 &&
                tokensEqual(aBinOp->left, bBinOp->left) &&
                tokensEqual(aBinOp->right, bBinOp->right);
    } else if(a->type == TOKEN_UNARY_OP) {
        UnaryOpToken* aUnOp = (UnaryOpToken*) a;
        UnaryOpToken* bUnOp = (UnaryOpToken*) b;
        return strcmp(aUnOp->op, bUnOp->op) == 0 &&
                tokensEqual(aUnOp->child, bUnOp->child);
    } else if(a->type == TOKEN_ASSIGNMENT) {
        AssignmentToken* assignA = (AssignmentToken*) a;
        AssignmentToken* assignB = (AssignmentToken*) b;
        return tokensEqual((Token*) assignA->left, (Token*) assignB->left) &&
                tokensEqual(assignB->right, assignB->right);
    } else if(a->type == TOKEN_BLOCK) {
        List* nodeA = ((BlockToken*) a)->children;
        List* nodeB = ((BlockToken*) b)->children;

        while(nodeA != NULL && nodeB != NULL) {
            if(!tokensEqual((Token*) nodeA->head, (Token*) nodeB->head)) {
                return 0;
            }
            nodeA = nodeA->tail;
            nodeB = nodeB->tail;
        }
        return nodeA == NULL && nodeB == NULL;
    } else if(a->type == TOKEN_IF) {
        IfToken* ifA = (IfToken*) a;
        IfToken* ifB = (IfToken*) b;

        List* nodeA = ifA->branches;
        List* nodeB = ifB->branches;

        while(nodeA != NULL && nodeB != NULL) {
            IfBranch* branchA = (IfBranch*) nodeA->head;
            IfBranch* branchB = (IfBranch*) nodeB->head;

            if(!tokensEqual((Token*) branchA->condition, (Token*) branchB->condition)) {
                return 0;
            }

            if(!tokensEqual((Token*) branchA->block, (Token*) branchB->block)) {
                return 0;
            }
            nodeA = nodeA->tail;
            nodeB = nodeB->tail;
        }
        if(nodeA != NULL || nodeB != NULL) {
            return 0;
        }
        if(ifA->elseBranch == NULL && ifA->elseBranch == NULL) {
            return 1;
        }
        return tokensEqual((Token*) ifA->elseBranch, (Token*) ifB->elseBranch);
    } else if(a->type == TOKEN_WHILE) {
        WhileToken* whileA = (WhileToken*) a;
        WhileToken* whileB = (WhileToken*) b;
        return tokensEqual((Token*) whileA->condition, (Token*) whileB->condition) &&
                tokensEqual((Token*) whileA->body, (Token*) whileB->body);
    } else if(a->type == TOKEN_FUNC) {
        FuncToken* funcA = (FuncToken*) a;
        FuncToken* funcB = (FuncToken*) b;
        return tokensEqual((Token*) funcA->name, (Token*) funcB->name) &&
                allList(funcA->args, funcB->args, tokensEqualVoid) &&
                tokensEqual((Token*) funcA->body, (Token*) funcB->body);
    } else {
        return 0;
    }
}

int tokensEqualVoid(void* a, void* b) {
    return tokensEqual((Token*) a, (Token*) b);
}

void printIndent(int indent) {
    for(int i = 0; i < indent; i++) {
        printf("    ");
    }
}

/**
 * Prints the given token. Subchildren are indented.
 */
void printToken(Token* token, int indent) {
    printIndent(indent);

    if(token == NULL) {
        printf("NULL\n");
    } else if(token->type == TOKEN_INT) {
        printf("int: %i\n", ((IntToken*) token)->value);
    } else if(token->type == TOKEN_LITERAL) {
        printf("literal: %s\n", ((LiteralToken*) token)->value);
    } else if(token->type == TOKEN_IDENTIFIER) {
        printf("identifier: %s\n", ((IdentifierToken*) token)->value);
    } else if(token->type == TOKEN_BINARY_OP) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) token;
        printf("binaryOp: '%s'\n", binaryOp->op);
        printToken(binaryOp->left, indent + 1);
        printToken(binaryOp->right, indent + 1);
    } else if(token->type == TOKEN_UNARY_OP) {
        UnaryOpToken* unaryOp = (UnaryOpToken*) token;
        printf("unaryOp: %s\n", unaryOp->op);
        printToken(unaryOp->child, indent + 1);
    } else if(token->type == TOKEN_ASSIGNMENT) {
        AssignmentToken* assignment = (AssignmentToken*) token;
        printf("assignment: %s\n", assignment->left->value);
        printToken(assignment->right, indent + 1);
    } else if(token->type == TOKEN_BLOCK) {
        BlockToken* block = (BlockToken*) token;
        printf("block:\n");
        for(List* node = block->children; node != NULL; node = node->tail) {
            printToken((Token*) node->head, indent + 1);
        }
    } else if(token->type == TOKEN_IF) {
        IfToken* ifStmt = (IfToken*) token;
        printf("if:\n");
        for(List* node = ifStmt->branches; node != NULL; node = node->tail) {
            IfBranch* branch = (IfBranch*) node->head;
            printIndent(indent + 1);
            printf("condition:\n");
            printToken((Token*) branch->condition, indent + 2);
            printIndent(indent + 1);
            printf("body:\n");
            printToken((Token*) branch->block, indent + 2);
        }
        printIndent(indent + 1);
        printf("else:\n");
        printToken((Token*) ifStmt->elseBranch, indent + 2);
    } else if(token->type == TOKEN_WHILE) {
        WhileToken* whileStmt = (WhileToken*) token;
        printf("while:\n");

        printIndent(indent + 1);
        printf("condition:\n");
        printToken((Token*) whileStmt->condition, indent + 2);

        printIndent(indent + 1);
        printf("body:\n");
        printToken((Token*) whileStmt->body, indent + 2);
    } else if(token->type == TOKEN_FUNC) {
        FuncToken* func = (FuncToken*) token;
        printf("func: %s", func->name->value);
        List* node = func->args;
        while(node != NULL) {
            printf(" %s", ((IdentifierToken*) node->head)->value);
            node = node->tail;
        }
        printf("\n");
        printToken((Token*) func->body, indent + 1);
    } else {
        printf("unknown\n");
    }
}

void printToken(Token* token) {
    printToken(token, 0);
}

void parseCleanup(ParseState* state, Token* token) {
    free(state);
    destroyToken(token);
}

const char* testParseInt() {
    ParseState* state = createParseState("1234");
    IntToken* token = parseInt(state);

    assert(token->token.type == TOKEN_INT, "token type is not int");
    assert(token->value == 1234, "token value is not 1234");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseLiteral() {
    ParseState* state = createParseState("'Hello World'");
    LiteralToken* token = parseLiteral(state);

    assert(token->token.type == TOKEN_LITERAL, "token type is not literal");
    assert(strcmp(token->value, "Hello World") == 0, "token value is not 'Hello World'");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseIdentifier() {
    ParseState* state = createParseState("var");
    IdentifierToken* token = parseIdentifier(state);

    assert(token->token.type == TOKEN_IDENTIFIER, "token type is not identifier");
    assert(strcmp(token->value, "var") == 0, "token value is not 'Hello'");

    parseCleanup(state, (Token*) token);
    return NULL;
}

const char* testParseExpression() {
    ParseState* state = createParseState("2 * ( a + 1 ) > 5 and f b == c or not d");
    Token* parsed = parseExpression(state);

    BinaryOpToken* expected = createBinaryOpToken(newStr("or"),
            (Token*) createBinaryOpToken(newStr("and"),
                    (Token*) createBinaryOpToken(newStr(">"),
                            (Token*) createBinaryOpToken(newStr("*"),
                                    (Token*) createIntToken(2),
                                    (Token*) createBinaryOpToken(newStr("+"),
                                            (Token*) createIdentifierToken(newStr("a")),
                                            (Token*) createIntToken(1))),
                            (Token*) createIntToken(5)),
                    (Token*) createBinaryOpToken(newStr("=="),
                            (Token*) createBinaryOpToken(newStr(" "),
                                    (Token*) createIdentifierToken(newStr("f")),
                                    (Token*) createIdentifierToken(newStr("b"))),
                            (Token*) createIdentifierToken(newStr("c")))),
            (Token*) createUnaryOpToken(newStr("not"),
                    (Token*) createIdentifierToken(newStr("d"))));

    assert(tokensEqual(parsed, (Token*) expected), "incorrect parse");

    free(state);
    destroyToken(parsed);
    destroyToken((Token*) expected);

    return NULL;
}

const char* testParseFail() {
    ParseState* state = createParseState("'hello");
    assert(parseExpression(state) == NULL, "parse succeeded for identifier");
    free(state);

    state = createParseState("1 + (a *");
    assert(parseExpression(state) == NULL, "parse succeeded for parenthesis");
    free(state);

    state = createParseState("2 * %");
    assert(parseExpression(state) == NULL, "parse succeeded, for unknown factor");
    free(state);
    return NULL;
}

const char* testParseAssignments() {
    BlockToken* parsed = parseModule("a = 1 + 2; b = 3; c;");

    AssignmentToken* stmt1 = createAssignmentToken(
            createIdentifierToken(newStr("a")),
            (Token*) createBinaryOpToken(newStr("+"),
                    (Token*) createIntToken(1),
                    (Token*) createIntToken(2)));

    AssignmentToken* stmt2 = createAssignmentToken(
            createIdentifierToken(newStr("b")),
            (Token*) createIntToken(3));

    IdentifierToken* stmt3 = createIdentifierToken(newStr("c"));

    List* list = consList(stmt1, consList(stmt2, consList(stmt3, NULL)));
    BlockToken* expected = createBlockToken(list);

    assert(tokensEqual((Token*) parsed, (Token*) expected), "incorrect parse");

    destroyToken((Token*) parsed);
    destroyToken((Token*) expected);
    return NULL;
}

const char* parseTestBlockWithEndFails() {
    BlockToken* parsed = parseModule("a = 1 + 2; b = 3; c; end");
    assert(parsed == NULL, "parse succeeded");
    return NULL;
}

const char* parseTestIfStmt() {
    BlockToken* parsed = parseModule("if a > b ; then c = 1; elif d ; then c = 2; else c = 3; end");

    IfBranch* branch1 = createIfBranch(
            createBlockToken(consList(
                createBinaryOpToken(newStr(">"),
                        (Token*) createIdentifierToken(newStr("a")),
                        (Token*) createIdentifierToken(newStr("b"))), NULL)),
            createBlockToken(consList(
                    createAssignmentToken(
                            createIdentifierToken(newStr("c")),
                            (Token*) createIntToken(1)), NULL)));


    IfBranch* branch2 = createIfBranch(
            createBlockToken(consList(
                (Token*) createIdentifierToken(newStr("d")), NULL)),
            createBlockToken(consList(
                createAssignmentToken(
                        createIdentifierToken(newStr("c")),
                        (Token*) createIntToken(2)), NULL)));

    BlockToken* elseBranch = createBlockToken(consList(
            createAssignmentToken(
                    createIdentifierToken(newStr("c")),
                    (Token*) createIntToken(3)), NULL));

    IfToken* ifToken = createIfToken(consList(branch1, consList(branch2, NULL)), elseBranch);
    BlockToken* expected = createBlockToken(consList(ifToken, NULL));
    assert(tokensEqual((Token*) parsed, (Token*) expected), "incorrect parse");

    destroyToken((Token*) parsed);
    destroyToken((Token*) expected);
    return NULL;
}

const char* parseTestWhileStmt() {
    Token* parsed = (Token*) parseModule("x = 0; while x < 10; do x = x + 1; end");

    Token* stmt1 = (Token*) createAssignmentToken(
            createIdentifierToken(newStr("x")),
            (Token*) createIntToken(0));

    Token* stmt2 = (Token*) createWhileToken(
            createBlockToken(consList(
                    createBinaryOpToken(newStr("<"),
                            (Token*) createIdentifierToken(newStr("x")),
                            (Token*) createIntToken(10)), NULL)),
            createBlockToken(consList(
                    createAssignmentToken(
                            createIdentifierToken(newStr("x")),
                            (Token*) createBinaryOpToken(newStr("x"),
                                    (Token*) createIdentifierToken(newStr("x")),
                                    (Token*) createIntToken(1))), NULL)));

    Token* expected = (Token*) createBlockToken(consList(stmt1, consList(stmt2, NULL)));
    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);
    return NULL;
}

const char* parseTestFunc() {
    Token* parsed = (Token*) parseModule("def add a b do a + b; end");

    List* args = consList(createIdentifierToken(newStr("a")),
            consList(createIdentifierToken(newStr("b")), NULL));

    BinaryOpToken* stmt = createBinaryOpToken(newStr("+"),
            (Token*) createIdentifierToken(newStr("a")),
            (Token*) createIdentifierToken(newStr("b")));

    FuncToken* func = createFuncToken(createIdentifierToken(newStr("add")), args,
            createBlockToken(consList(stmt, NULL)));

    Token* expected = (Token*) createBlockToken(consList(func, NULL));

    assert(tokensEqual(parsed, expected), "incorrect parse");

    destroyToken(parsed);
    destroyToken(expected);

    return NULL;
}
