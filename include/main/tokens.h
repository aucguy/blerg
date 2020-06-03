#ifndef TOKENS_H_
#define TOKENS_H_

#include <stdint.h>

#include "main/flags.h"
#include "main/util.h"

/**
 * Types of different tokens. Used to determine the type of tokens during
 * runtime.
 */
typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_TUPLE,
    TOKEN_LIST,
    TOKEN_OBJECT,
    TOKEN_CALL,
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
    TOKEN_COND_JUMP,
    TOKEN_PUSH_BUILTIN,
    TOKEN_PUSH_INT,
    TOKEN_OP_CALL,
    TOKEN_STORE,
    TOKEN_DUP,
    TOKEN_PUSH,
    TOKEN_ROT3,
    TOKEN_SWAP,
    TOKEN_BUILTIN,
    TOKEN_CHECK_NONE
} TokenType;

typedef struct Token Token;

typedef struct Token* (*CopyVisitor)(struct Token*, void*);

/**
 * Supertype for all tokens. This should be the first field of all tokens,
 * so tokens can be casted to their subtype and this supertype. All fields of
 * the token must be unique references.
 */
struct Token {
    //class fields
    TokenType type;
    void (*destroy)(struct Token*);
    void (*print)(struct Token*, uint8_t);
    uint8_t (*equals)(struct Token*, struct Token*);
    struct Token* (*copy)(struct Token*, CopyVisitor, void*);

    //instance fields
    SrcLoc location;
};

typedef struct {
    Token token;
    int32_t value;
} IntToken;

typedef struct {
    Token token;
    float value;
} FloatToken;

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
    List* elements;
} TupleToken;

typedef struct {
    Token token;
    List* elements;
} ListToken;

typedef struct {
    Token* key;
    Token* value;
} ObjectPair;

typedef struct {
    Token token;
    List* elements;
} ObjectToken;

typedef struct {
    Token token;
    List* children;
} CallToken;

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
    Token* left;
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
    uint8_t when;
} CondJumpToken;

typedef struct {
    Token token;
    const char* name;
} PushBuiltinToken;

typedef struct {
    Token token;
    int32_t value;
} PushIntToken;

typedef struct {
    Token token;
    uint8_t arity;
} CallOpToken;

typedef struct {
    Token token;
    const char* name;
} StoreToken;

typedef struct {
  Token token;
} DupToken;

typedef struct {
    Token token;
    Token* value;
} PushToken;

typedef struct {
    Token token;
} Rot3Token;

typedef struct {
    Token token;
} SwapToken;

typedef struct {
    Token token;
    const char* name;
} BuiltinToken;

typedef struct {
    Token token;
} CheckNoneToken;

void printTokenWithIndent(Token* token, uint8_t indent);
void printToken(Token* token);
void printIndent(uint8_t indent);

/**
 * Frees the token, its fields and subtokens.
 */
void destroyToken(Token* token);
void destroyTokenVoid(void*);
void destroyIfBranch(void*);

uint8_t tokensEqual(Token* a, Token* b);

Token* copyToken(Token*, CopyVisitor, void*);

#if INCLUDE_TESTS
IntToken* createIntToken(SrcLoc, int32_t);
FloatToken* createFloatToken(SrcLoc, float);
LiteralToken* createLiteralToken(SrcLoc, const char*);
IdentifierToken* createIdentifierToken(SrcLoc, const char*);
TupleToken* createTupleToken(SrcLoc, List*);
ListToken* createListToken(SrcLoc, List*);
ObjectPair* createObjectPair(Token*, Token*);
ObjectToken* createObjectToken(SrcLoc, List*);
CallToken* createCallToken(SrcLoc, List*);
BinaryOpToken* createBinaryOpToken(SrcLoc, const char*, Token*, Token*);
UnaryOpToken* createUnaryOpToken(SrcLoc, const char*, Token*);
AssignmentToken* createAssignmentToken(SrcLoc, Token*, Token*);
BlockToken* createBlockToken(SrcLoc, List*);
IfBranch* createIfBranch(Token*, BlockToken*);
IfToken* createIfToken(SrcLoc, List*, BlockToken*);
WhileToken* createWhileToken(SrcLoc, Token*, BlockToken*);
FuncToken* createFuncToken(SrcLoc, IdentifierToken*, List*, BlockToken*);
ReturnToken* createReturnToken(SrcLoc, Token* body);
#endif

LabelToken* createLabelToken(const char*);
AbsJumpToken* createAbsJumpToken(const char*);
CondJumpToken* createCondJumpToken(SrcLoc, Token*, const char*, uint8_t);
PushBuiltinToken* createPushBuiltinToken(SrcLoc, const char* name);
PushIntToken* createPushIntToken(SrcLoc, int32_t value);
CallOpToken* createCallOpToken(SrcLoc, uint8_t arity);
StoreToken* createStoreToken(SrcLoc, const char* name);
DupToken* createDupToken(SrcLoc);
PushToken* createPushToken(SrcLoc, Token*);
Rot3Token* createRot3Token(SrcLoc);
SwapToken* createSwapToken(SrcLoc);
BuiltinToken* createBuiltinToken(SrcLoc loc, const char* name);
CheckNoneToken* createCheckNoneToken(SrcLoc loc);

#endif /* TOKENS_H_ */
