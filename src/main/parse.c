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

IntToken* parseInt(ParseState* state) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token.type = TOKEN_INT;

    int start = state->index;
    while(strchr("123456789", getChar(state)) != NULL) {
        advance(state);
    }

    int len = sizeof(char) * (state->index - start);
    char* extracted = (char*) malloc(len);
    memcpy(extracted, &state->src[start], len);
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

    int len = sizeof(char) * (state->index - start + 1);
    char* extracted = (char*) malloc(len);
    memcpy(extracted, &state->src[start], len);
    extracted[len - 1] = 0;
    token->value = extracted;

    advance(state); //skip the '

    return token;
}

void destroyToken(Token* token) {
    if(token->type == TOKEN_LITERAL) {
        free((void*) ((LiteralToken*) token)->value);
    }
    free(token);
}
