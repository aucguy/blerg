#ifndef PARSE_H_
#define PARSE_H_

#include "main/flags.h"
#include "main/util.h"

/**
 * Types of different tokens. Used to determine the type of tokens during
 * runtime.
 */
typedef enum {
    TOKEN_INT,
    TOKEN_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_BINARY_OP,
    TOKEN_UNARY_OP,
    TOKEN_ASSIGNMENT,
    TOKEN_BLOCK
} TokenType;

/**
 * Supertype for all tokens. This should be the first field of all tokens,
 * so tokens can be casted to their subtype and this supertype. All fields of
 * the token must be unique references.
 */
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

typedef struct {
    Token token;
    const char* op;
    Token* child;
} UnaryOpToken;

typedef struct {
    Token token;
    IdentifierToken* left;
    Token* right;
} AssignmentToken;

typedef struct {
    Token token;
    List* children;
} BlockToken;

/**
 * Frees the token, its fields and subtokens.
 */
void destroyToken(Token* token);

#if INCLUDE_TESTS || IS_PARSE_IMPL
/**
 * Holds the source code and the current parse position in it. The parsing
 * functions increment the index as they consume tokens.
 */
typedef struct {
    const char* src;
    int index;
} ParseState;

/**
 * Creates a ParseState from the given source code.
 */
ParseState* createParseState(const char*);
#endif

#if INCLUDE_TESTS
IntToken* createIntToken(int);
LiteralToken* createLiteralToken(const char*);
IdentifierToken* createIdentifierToken(const char*);
BinaryOpToken* createBinaryOpToken(const char*, Token*, Token*);
UnaryOpToken* createUnaryOpToken(const char*, Token*);
AssignmentToken* createAssignmentToken(IdentifierToken*, Token*);
BlockToken* createBlockToken(List*);

IntToken* parseInt(ParseState*);
LiteralToken* parseLiteral(ParseState*);
IdentifierToken* parseIdentifier(ParseState*);
Token* parseTerm(ParseState*);
Token* parseExpression(ParseState*);
BlockToken* parseBlock(ParseState*);
#endif

#endif /* PARSE_H_ */
