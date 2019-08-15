#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test/parseTest.h"
#include "main/parse.h"

int tokensEqual(Token* a, Token* b) {
    if(a->type != b->type) {
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
    } else {
        return 0;
    }
}

const char* newStr(const char* src) {
    int len = sizeof(char) * (strlen(src) + 1);
    char* dst = (char*) malloc(len);
    memcpy(dst, src, len);
    dst[len - 1] = 0;
    return dst;
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

const char* testParseTerm() {
    ParseState* state = createParseState("1*'hello'/foo");
    Token* parsed = parseTerm(state);

    BinaryOpToken* expected = createBinaryOpToken(newStr("/"),
            (Token*) createBinaryOpToken(newStr("*"),
                    (Token*) createIntToken(1),
                    (Token*) createLiteralToken(newStr("hello"))),
            (Token*) createIdentifierToken(newStr("foo")));

    assert(tokensEqual(parsed, (Token*) expected), "incorrect parse");

    free(state);
    destroyToken((Token*) expected);
    destroyToken(parsed);

    return NULL;
}
