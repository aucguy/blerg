#ifndef TOKENS_H_
#define TOKENS_H_

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
    TOKEN_FUNC,
    TOKEN_RETURN,

    //are not generated via parsing, but are created during transformation
    //passes
    TOKEN_LABEL,
    TOKEN_ABS_JUMP,
    TOKEN_COND_JUMP
} TokenType;

/**
 * Supertype for all tokens. This should be the first field of all tokens,
 * so tokens can be casted to their subtype and this supertype. All fields of
 * the token must be unique references.
 */
typedef struct Token_ {
    TokenType type;
    void (*destroy)(struct Token_*);
    void (*print)(struct Token_*, int);
    int (*equals)(struct Token_*, struct Token_*);
    struct Token_* (*copy)(struct Token_*);
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
    Token* condition;
    BlockToken* block;
} IfBranch;

typedef struct {
    Token token;
    List* branches;
    BlockToken* elseBranch;
} IfToken;

typedef struct {
    Token token;
    Token* condition;
    BlockToken* body;
} WhileToken;

typedef struct {
    Token token;
    IdentifierToken* name;
    List* args;
    BlockToken* body;
} FuncToken;

typedef struct {
    Token token;
    Token* body;
} ReturnToken;

typedef struct {
    Token token;
    const char* name;
} LabelToken;

typedef struct {
    Token token;
    const char* label;
} AbsJumpToken;

typedef struct {
    Token token;
    Token* condition;
    const char* label;
    //if the condition is false and when is 0 then the jump is taken
    //if the condition is true and when is 1 then the jump is taken
    int when;
} CondJumpToken;

void printToken(Token* token, int indent);
void printToken(Token* token);
void printIndent(int indent);

/**
 * Frees the token, its fields and subtokens.
 */
void destroyToken(Token* token);
void destroyTokenVoid(void*);
void destroyIfBranch(void*);

int tokensEqual(Token* a, Token* b);

Token* copyToken(Token*);

#if INCLUDE_TESTS
IntToken* createIntToken(int);
LiteralToken* createLiteralToken(const char*);
IdentifierToken* createIdentifierToken(const char*);
BinaryOpToken* createBinaryOpToken(const char*, Token*, Token*);
UnaryOpToken* createUnaryOpToken(const char*, Token*);
AssignmentToken* createAssignmentToken(IdentifierToken*, Token*);
BlockToken* createBlockToken(List*);
IfBranch* createIfBranch(Token*, BlockToken*);
IfToken* createIfToken(List*, BlockToken*);
WhileToken* createWhileToken(Token*, BlockToken*);
FuncToken* createFuncToken(IdentifierToken*, List*, BlockToken*);
ReturnToken* createReturnToken(Token* body);
#endif

LabelToken* createLabelToken(const char*);
AbsJumpToken* createAbsJumpToken(const char*);
CondJumpToken* createCondJumpToken(Token*, const char*, int);

#endif /* TOKENS_H_ */
