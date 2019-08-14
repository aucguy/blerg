#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test/parseTest.h"
#include "main/parse.h"

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

const char* testParseTerm() {
    ParseState* state = createParseState("1*'hello'/foo");
    Token* token = parseTerm(state);

    assert(token->type == TOKEN_BINARY_OP, "second op is not a binary op");
    BinaryOpToken* outerOp = (BinaryOpToken*) token;
    assert(strcmp(outerOp->op, "/") == 0, "second op is not '/'");

    assert(outerOp->right->type == TOKEN_IDENTIFIER, "third factor is not an identifier");
    IdentifierToken* third = (IdentifierToken*) outerOp->right;
    assert(strcmp(third->value, "foo") == 0, "third factor is not 'foo'");

    assert(outerOp->left->type == TOKEN_BINARY_OP, "second op is not a binary op");
    BinaryOpToken* innerOp = (BinaryOpToken*) outerOp->left;
    assert(strcmp(innerOp->op, "*") == 0, "first op is not '*'");

    assert(innerOp->left->type == TOKEN_INT, "first factor is not an int");
    IntToken* first = (IntToken*) innerOp->left;
    assert(first->value == 1, "first factor is not 1");

    assert(innerOp->right->type == TOKEN_LITERAL, "second factor is not a literal");
    LiteralToken* second = (LiteralToken*) innerOp->right;
    assert(strcmp(second->value, "hello") == 0, "second factor is not 'hello'");

    free(state);
    destroyToken(token);

    return NULL;
}
