#define IS_PARSE_IMPL 1
#include <stdlib.h>
#include <string.h>
#include "main/parse.h"

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
    while(strchr(chars, getChar(state)) != NULL) {
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
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token.type = TOKEN_INT;

    int start = state->index;
    advanceWhile(state, INT_CHARS);

    char* extracted = sliceStr(state->src, start, state->index);
    token->value = atoi(extracted);
    free(extracted);

    return token;
}


LiteralToken* parseLiteral(ParseState* state) {
    LiteralToken* token = (LiteralToken*) malloc(sizeof(LiteralToken));
    token->token.type = TOKEN_LITERAL;

    advance(state); //skip the '
    int start = state->index;
    while(getChar(state) != '\'') {
        advance(state);
    }

    token->value = sliceStr(state->src, start, state->index);
    advance(state); //skip the '

    return token;
}

const char* IDENTIFIER_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

IdentifierToken* parseIdentifier(ParseState* state) {
    IdentifierToken* token = (IdentifierToken*) malloc(sizeof(IdentifierToken));
    token->token.type = TOKEN_IDENTIFIER;

    int start = state->index;
    advanceWhile(state, IDENTIFIER_CHARS);

    token->value = sliceStr(state->src, start, state->index);
    return token;
}

Token* parseFactor(ParseState* state) {
    char c = getChar(state);
    if(strchr(INT_CHARS, c) != NULL) {
        return (Token*) parseInt(state);
    } else if(strchr(IDENTIFIER_CHARS, c) != NULL) {
        return (Token*) parseIdentifier(state);
    } else if(c == '\'') {
        return (Token*) parseLiteral(state);
    } else {
        return NULL;
    }
}

Token* parseTerm(ParseState* state) {
    Token* token = parseFactor(state);

    while(strchr("*/", getChar(state)) != NULL) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) malloc(sizeof(BinaryOpToken));
        binaryOp->token.type = TOKEN_BINARY_OP;
        binaryOp->op = sliceStr(state->src, state->index, state->index + 1);
        advance(state);
        binaryOp->left = token;
        binaryOp->right = parseFactor(state);
        token = (Token*) binaryOp;
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
