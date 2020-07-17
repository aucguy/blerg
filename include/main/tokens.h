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

/**
 * Supertype for all tokens. This should be the first field of all tokens,
 * so tokens can be casted to their subtype and this supertype. All fields of
 * the token must be unique references.
 */
class Token : public TokenMethods {
public:
    SrcLoc location;
    //TokenMethods* methods;

    //Token(SrcLoc location) : location(location), methods(this) {}
    Token(SrcLoc location) : location(location) {}

    virtual TokenType type() {
        return TOKEN_UNDEF;
    }

    virtual void destroy(Token*) {}

    virtual void print(Token*, uint8_t) {};

    virtual uint8_t equals(Token*, Token*) {
        return 0;
    }

    virtual Token* copy(Token*, CopyVisitor, void*) {
        return nullptr;
    }
};

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
    //List* branches_;
    //BlockToken* elseBranch_;
} IfToken;

List* getIfTokenBranches(IfToken* token);
BlockToken* getIfTokenElseBranch(IfToken* token);

typedef struct {
    Token token_;
    //Token* condition_;
    //BlockToken* body_;
} WhileToken;

Token* getWhileTokenCondition(WhileToken* token);
BlockToken* getWhileTokenBody(WhileToken* token);

typedef struct {
    Token token_;
    //IdentifierToken* name_;
    //List* args_;
    //BlockToken* body_;
} FuncToken;

IdentifierToken* getFuncTokenName(FuncToken* token);
void setFuncTokenName(FuncToken* token, IdentifierToken* name);
List* getFuncTokenArgs(FuncToken* token);
BlockToken* getFuncTokenBody(FuncToken* token);

typedef struct {
    Token token_;
    //Token* body_;
} ReturnToken;

Token* getReturnTokenBody(ReturnToken* token);

typedef struct {
    Token token_;
    //const char* name_;
} LabelToken;

const char* getLabelTokenName(LabelToken* token);

typedef struct {
    Token token_;
    //const char* label_;
} AbsJumpToken;

const char* getAbsJumpTokenLabel(AbsJumpToken* token);

typedef struct {
    Token token_;
    //Token* condition_;
    //const char* label_;
    //if the condition is false and when is 0 then the jump is taken
    //if the condition is true and when is 1 then the jump is taken
    //uint8_t when_;
} CondJumpToken;

Token* getCondJumpTokenCondition(CondJumpToken* token);
const char* getCondJumpTokenLabel(CondJumpToken* token);
uint8_t getCondJumpTokenWhen(CondJumpToken* token);

typedef struct {
    Token token_;
    //const char* name_;
} PushBuiltinToken;

const char* getPushBuiltinTokenName(PushBuiltinToken* token);

typedef struct {
    Token token_;
    //int32_t value_;
} PushIntToken;

int32_t getPushIntTokenValue(PushIntToken* token);

typedef struct {
    Token token_;
    //uint8_t arity_;
} CallOpToken;

uint8_t getCallOpTokenArity(CallOpToken* token);

typedef struct {
    Token token_;
    //const char* name_;
} StoreToken;

const char* getStoreTokenName(StoreToken* token);

typedef struct {
    Token token_;
} DupToken;

typedef struct {
    Token token_;
    //Token* value_;
} PushToken;

Token* getPushTokenValue(PushToken* token);

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
    //const char* name_;
} BuiltinToken;

const char* getBuiltinTokenName(BuiltinToken* token);

typedef struct {
    Token token_;
} CheckNoneToken;

typedef struct {
    Token token_;
    //const char* name_;
} NewFuncToken;

const char* getNewFuncTokenName(NewFuncToken* token);

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
