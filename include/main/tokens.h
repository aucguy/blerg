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
    TOKEN_POP,
    TOKEN_BUILTIN,
    TOKEN_CHECK_NONE,
    TOKEN_NEW_FUNC,
    TOKEN_UNDEF
} TokenType;

typedef struct Token Token;

typedef struct Token* (*CopyVisitor)(struct Token*, void*);

class TokenMethods {
public:
    virtual ~TokenMethods() {} ;
    virtual TokenType type() = 0;
    virtual void destroy(Token*) = 0;
    virtual void print(Token*, uint8_t) = 0;
    virtual uint8_t equals(Token*, Token*) = 0;
    virtual Token* copy(Token*, CopyVisitor, void*) = 0;
};

class LegacyTokenMethods : public TokenMethods {
public:
    TokenType typeValue;
    void (*destroyFunc)(Token*);
    void (*printFunc)(Token*, uint8_t);
    uint8_t (*equalsFunc)(Token*, Token*);
    Token* (*copyFunc)(Token*, CopyVisitor, void*);

    LegacyTokenMethods();
    ~LegacyTokenMethods();
    TokenType type();
    void destroy(Token*);
    void print(Token*, uint8_t);
    uint8_t equals(Token*, Token*);
    Token* copy(Token*, CopyVisitor, void*);
};
/**
 * Supertype for all tokens. This should be the first field of all tokens,
 * so tokens can be casted to their subtype and this supertype. All fields of
 * the token must be unique references.
 */
struct Token {
    TokenMethods* methods;

    //class fields
    //TokenType type_;
    //void (*destroy)(struct Token*);
    //void (*print)(struct Token*, uint8_t);
    //uint8_t (*equals)(struct Token*, struct Token*);
    //struct Token* (*copy)(struct Token*, CopyVisitor, void*);

    //instance fields
    SrcLoc location_;
};

typedef struct {
    TokenType type;
    void (*destroy)(struct Token*);
    void (*print)(struct Token*, uint8_t);
    uint8_t (*equals)(struct Token*, struct Token*);
    struct Token* (*copy)(struct Token*, CopyVisitor, void*);
} LegacyTokenInit;

Token createLegacyTokenType(LegacyTokenInit);

SrcLoc tokenLocation(Token* token);
void setTokenLocation(Token* token, SrcLoc loc);
TokenType getTokenType(Token* token);
void setTokenType(Token* token, Token type);

typedef struct {
    Token token_;
    //int32_t value;
} IntToken;

int32_t getIntTokenValue(IntToken* token);

typedef struct {
    Token token_;
    //float value;
} FloatToken;

float getFloatTokenValue(FloatToken* token);

typedef struct {
    Token token_;
    //const char* value_;
} LiteralToken;

const char* getLiteralTokenValue(LiteralToken* token);

typedef struct {
    Token token_;
    //const char* value_;
} IdentifierToken;

const char* getIdentifierTokenValue(IdentifierToken* token);

typedef struct {
    Token token_;
    //List* elements_;
} TupleToken;

List* getTupleTokenElements(TupleToken* token);

typedef struct {
    Token token_;
    //List* elements_;
} ListToken;

List* getListTokenElements(ListToken* token);

typedef struct {
    Token* key;
    Token* value;
} ObjectPair;

typedef struct {
    Token token_;
    //List* elements_;
} ObjectToken;

List* getObjectTokenElements(ObjectToken* token);

typedef struct {
    Token token_;
    //List* children_;
} CallToken;

List* getCallTokenChildren(CallToken* token);

typedef struct {
    Token token_;
    //const char* op_;
    //Token* left_;
    //Token* right_;
} BinaryOpToken;

const char* getBinaryOpTokenOp(BinaryOpToken* token);
Token* getBinaryOpTokenLeft(BinaryOpToken* token);
Token* getBinaryOpTokenRight(BinaryOpToken* token);

typedef struct {
    Token token_;
    //const char* op_;
    //Token* child_;
} UnaryOpToken;

const char* getUnaryOpTokenOp(UnaryOpToken* token);
Token* getUnaryOpTokenChild(UnaryOpToken* token);

typedef struct {
    Token token_;
    //Token* left_;
    //Token* right_;
} AssignmentToken;

Token* getAssignmentTokenLeft(AssignmentToken* token);
Token* getAssignmentTokenRight(AssignmentToken* token);

typedef struct {
    Token token_;
    //List* children_;
} BlockToken;

List* getBlockTokenChildren(BlockToken* token);

typedef struct {
    Token* condition;
    BlockToken* block;
} IfBranch;

typedef struct {
    Token token_;
    List* branches;
    BlockToken* elseBranch;
} IfToken;

typedef struct {
    Token token_;
    Token* condition;
    BlockToken* body;
} WhileToken;

typedef struct {
    Token token_;
    IdentifierToken* name;
    List* args;
    BlockToken* body;
} FuncToken;

typedef struct {
    Token token_;
    Token* body;
} ReturnToken;

typedef struct {
    Token token_;
    const char* name;
} LabelToken;

typedef struct {
    Token token_;
    const char* label;
} AbsJumpToken;

typedef struct {
    Token token_;
    Token* condition;
    const char* label;
    //if the condition is false and when is 0 then the jump is taken
    //if the condition is true and when is 1 then the jump is taken
    uint8_t when;
} CondJumpToken;

typedef struct {
    Token token_;
    const char* name;
} PushBuiltinToken;

typedef struct {
    Token token_;
    int32_t value;
} PushIntToken;

typedef struct {
    Token token_;
    uint8_t arity;
} CallOpToken;

typedef struct {
    Token token_;
    const char* name;
} StoreToken;

typedef struct {
    Token token_;
} DupToken;

typedef struct {
    Token token_;
    Token* value;
} PushToken;

typedef struct {
    Token token_;
} Rot3Token;

typedef struct {
    Token token_;
} SwapToken;

typedef struct {
    Token token_;
} PopToken;

typedef struct {
    Token token_;
    const char* name;
} BuiltinToken;

typedef struct {
    Token token_;
} CheckNoneToken;

typedef struct {
    Token token_;
    const char* name;
} NewFuncToken;

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
PopToken* createPopToken(SrcLoc);
BuiltinToken* createBuiltinToken(SrcLoc loc, const char* name);
CheckNoneToken* createCheckNoneToken(SrcLoc loc);
NewFuncToken* createNewFuncToken(SrcLoc loc, const char* name);

#endif /* TOKENS_H_ */
