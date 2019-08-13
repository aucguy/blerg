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

IntToken* parseInt(ParseState* state) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token.type = TOKEN_INT;

    int start = state->index;
    advanceWhile(state, "123456789");

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

void destroyToken(Token* token) {
    if(token->type == TOKEN_LITERAL) {
        free((void*) ((LiteralToken*) token)->value);
    } else if(token->type == TOKEN_IDENTIFIER) {
        free((void*) ((IdentifierToken*) token)->value);
    }
    free(token);
}
