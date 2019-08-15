#define IS_PARSE_IMPL 1
#include <stdlib.h>
#include <string.h>
#include "main/parse.h"

int containsChar(const char* chars, char c) {
    return c != 0 && strchr(chars, c);
}

IntToken* createIntToken(int value) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token.type = TOKEN_INT;
    token->value = value;
    return token;
}

LiteralToken* createLiteralToken(const char* value) {
    LiteralToken* token = (LiteralToken*) malloc(sizeof(LiteralToken));
    token->token.type = TOKEN_LITERAL;
    token->value = value;
    return token;
}

IdentifierToken* createIdentifierToken(const char* value) {
    IdentifierToken* token = (IdentifierToken*) malloc(sizeof(IdentifierToken));
    token->token.type = TOKEN_IDENTIFIER;
    token->value = value;
    return token;
}

BinaryOpToken* createBinaryOpToken(const char* op, Token* left, Token* right) {
    BinaryOpToken* token = (BinaryOpToken*) malloc(sizeof(BinaryOpToken));
    token->token.type = TOKEN_BINARY_OP;
    token->op = op;
    token->left = left;
    token->right = right;
    return token;
}

ParseState* createParseState(const char* src) {
    ParseState* state = (ParseState*) malloc(sizeof(ParseState));
    state->index = 0;
    state->src = src;
    return state;
}

char getChar(ParseState* state) {
    return state->src[state->index];
}

void advance(ParseState* state) {
    state->index++;
}

void advanceWhile(ParseState* state, const char* chars) {
    while(containsChar(chars, getChar(state)) != NULL) {
        advance(state);
    }
}

char* sliceStr(const char* str, int start, int end) {
    int len = sizeof(char) * (end - start);
    char* extracted = (char*) malloc(len + 1);
    memcpy(extracted, &str[start], len);
    extracted[len] = 0;
    return extracted;
}

const char* INT_CHARS = "0123456789";

IntToken* parseInt(ParseState* state) {
    int start = state->index;
    advanceWhile(state, INT_CHARS);

    char* extracted = sliceStr(state->src, start, state->index);
    int value = atoi(extracted);
    free(extracted);

    return createIntToken(value);
}


LiteralToken* parseLiteral(ParseState* state) {
    advance(state); //skip the '
    int start = state->index;
    while(getChar(state) != '\'') {
        advance(state);
    }

    const char* value = sliceStr(state->src, start, state->index);
    advance(state); //skip the '

    return createLiteralToken(value);
}

const char* IDENTIFIER_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

IdentifierToken* parseIdentifier(ParseState* state) {
    int start = state->index;
    advanceWhile(state, IDENTIFIER_CHARS);

    return createIdentifierToken(sliceStr(state->src, start, state->index));
}

Token* parseFactor(ParseState* state) {
    char c = getChar(state);
    if(containsChar(INT_CHARS, c) != NULL) {
        return (Token*) parseInt(state);
    } else if(containsChar(IDENTIFIER_CHARS, c) != NULL) {
        return (Token*) parseIdentifier(state);
    } else if(c == '\'') {
        return (Token*) parseLiteral(state);
    } else {
        return NULL;
    }
}

Token* parseTerm(ParseState* state) {
    Token* token = parseFactor(state);

    while(containsChar("*/", getChar(state)) != NULL) {
        const char* op = sliceStr(state->src, state->index, state->index + 1);
        advance(state);
        Token* right = parseFactor(state);
        token = (Token*) createBinaryOpToken(op, token, right);
    }

    return token;
}

void destroyToken(Token* token) {
    if(token->type == TOKEN_LITERAL) {
        free((void*) ((LiteralToken*) token)->value);
    } else if(token->type == TOKEN_IDENTIFIER) {
        free((void*) ((IdentifierToken*) token)->value);
    } else if(token->type == TOKEN_BINARY_OP) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) token;
        free((void*) binaryOp->op);
        destroyToken(binaryOp->left);
        destroyToken(binaryOp->right);
    }
    free(token);
}
