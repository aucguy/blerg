#ifndef PARSE_H_
#define PARSE_H_

#include "main/flags.h"

typedef enum {
    TOKEN_INT,
    TOKEN_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_BINARY_OP
} TokenType;

typedef struct {
    TokenType type;
} Token;

typedef struct {
    Token token;
    int value;
} IntToken;

typedef struct {
    Token token;
    const char* value;
} LiteralToken;

typedef struct {
    Token token;
    const char* value;
} IdentifierToken;

typedef struct {
    Token token;
    const char* op;
    Token* left;
    Token* right;
} BinaryOpToken;

void destroyToken(Token* token);

#if INCLUDE_TESTS || IS_PARSE_IMPL
typedef struct {
    const char* src;
    int index;
} ParseState;

ParseState* createParseState(const char*);
#endif

#if INCLUDE_TESTS
IntToken* parseInt(ParseState*);
LiteralToken* parseLiteral(ParseState*);
IdentifierToken* parseIdentifier(ParseState*);
Token* parseTerm(ParseState*);
#endif

#endif /* PARSE_H_ */
