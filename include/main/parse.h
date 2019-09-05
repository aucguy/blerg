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
    TOKEN_BLOCK,
    TOKEN_IF,
    TOKEN_WHILE,
    TOKEN_FUNC
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

typedef struct {
    BlockToken* condition;
    BlockToken* block;
} IfBranch;

typedef struct {
    Token token;
    List* branches;
    BlockToken* elseBranch;
} IfToken;

typedef struct {
    Token token;
    BlockToken* condition;
    BlockToken* body;
} WhileToken;

typedef struct {
    Token token;
    IdentifierToken* name;
    List* args;
    BlockToken* body;
} FuncToken;

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

/**
 * Parses the module
 *
 * @param src the source code for the given module
 * @return the AST for the source
 */
BlockToken* parseModule(const char* src);

#endif

#if INCLUDE_TESTS
IntToken* createIntToken(int);
LiteralToken* createLiteralToken(const char*);
IdentifierToken* createIdentifierToken(const char*);
BinaryOpToken* createBinaryOpToken(const char*, Token*, Token*);
UnaryOpToken* createUnaryOpToken(const char*, Token*);
AssignmentToken* createAssignmentToken(IdentifierToken*, Token*);
BlockToken* createBlockToken(List*);
IfBranch* createIfBranch(BlockToken*, BlockToken*);
IfToken* createIfToken(List*, BlockToken*);
WhileToken* createWhileToken(BlockToken*, BlockToken*);
FuncToken* createFuncToken(IdentifierToken*, List*, BlockToken*);

IntToken* parseInt(ParseState*);
LiteralToken* parseLiteral(ParseState*);
IdentifierToken* parseIdentifier(ParseState*);
Token* parseTerm(ParseState*);
Token* parseExpression(ParseState*);
#endif

#endif /* PARSE_H_ */
