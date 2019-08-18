#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test/parseTest.h"
#include "main/parse.h"
#include "main/util.h"

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
    } else {
        return 0;
    }
}

void printToken(Token* token, int indent) {
    for(int i = 0; i < indent; i++) {
        printf("    ");
    }

    if(token->type == TOKEN_INT) {
        printf("int: %i\n", ((IntToken*) token)->value);
    } else if(token->type == TOKEN_LITERAL) {
        printf("literal: %s\n", ((LiteralToken*) token)->value);
    } else if(token->type == TOKEN_IDENTIFIER) {
        printf("identifier: %s\n", ((IdentifierToken*) token)->value);
    } else if(token->type == TOKEN_BINARY_OP) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) token;
        printf("binaryOp: %s\n", binaryOp->op);
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
    ParseState* state = createParseState("2 * ( a + 1 ) > 5 and b == c or not d");
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
                            (Token*) createIdentifierToken(newStr("b")),
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
    ParseState* state = createParseState("a = 1 + 2; b = 3; c; end");
    BlockToken* parsed = parseBlock(state);

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

    free(state);
    destroyToken((Token*) parsed);
    destroyToken((Token*) expected);
    return NULL;
}
