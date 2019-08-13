#ifndef PARSE_H_
#define PARSE_H_

#include "main/flags.h"

typedef enum {
    TOKEN_INT
} TokenType;

typedef struct {
    TokenType type;
} Token;

typedef struct {
    Token token;
    int value;
} IntToken;

#if INCLUDE_TESTS || IS_PARSE_IMPL
typedef struct {
    const char* src;
    int index;
} ParseState;

ParseState* createParseState(const char*);
#endif

#if INCLUDE_TESTS
IntToken* parseInt(ParseState*);
#endif

#endif /* PARSE_H_ */
