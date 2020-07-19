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
    TOKEN_NEW_FUNC
} TokenType;

typedef struct Token Token;

typedef struct Token* (*CopyVisitor)(struct Token*, void*);

/**
 * Supertype for all tokens. This should be the first field of all tokens,
 * so tokens can be casted to their subtype and this supertype. All fields of
 * the token must be unique references.
 */
class Token {
public:
    SrcLoc location;

    Token(SrcLoc location) : location(location) {}
    virtual TokenType type() = 0;
    virtual void destroy(Token*) = 0;
    virtual void print(Token*, uint8_t) = 0;
    virtual uint8_t equals(Token*, Token*) = 0;
    virtual Token* copy(Token*, CopyVisitor, void*) = 0;
};

SrcLoc tokenLocation(Token* token);
//void setTokenLocation(Token* token, SrcLoc loc);
TokenType getTokenType(Token* token);

class IntToken : public Token {
public:
    int32_t value;

    IntToken(SrcLoc location, int32_t value);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* token, CopyVisitor visitor, void* data);
};

int32_t getIntTokenValue(IntToken* token);

class FloatToken : public Token {
public:
    float value;

    FloatToken(SrcLoc location, float value);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

float getFloatTokenValue(FloatToken* token);

class LiteralToken : public Token {
public:
    const char* value;

    LiteralToken(SrcLoc location, const char* value);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* token, CopyVisitor visitor, void* data);
};

const char* getLiteralTokenValue(LiteralToken* token);

class IdentifierToken : public Token {
public:
    const char* value;

    IdentifierToken(SrcLoc location, const char* value);
    TokenType type();
    void destroy(Token* token);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* token, CopyVisitor visitor, void* data);
};

const char* getIdentifierTokenValue(IdentifierToken* token);

class TupleToken : public Token {
public:
    List* elements;

    TupleToken(SrcLoc location, List* elements);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

List* getTupleTokenElements(TupleToken* token);

class ListToken : public Token {
public:
    List* elements;

    ListToken(SrcLoc location, List* elements);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

List* getListTokenElements(ListToken* token);

typedef struct {
    Token* key;
    Token* value;
} ObjectPair;


class ObjectToken : public Token {
public:
    List* elements;

    ObjectToken(SrcLoc location, List* elements);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

List* getObjectTokenElements(ObjectToken* token);

class CallToken : public Token {
public:
    List* children;

    CallToken(SrcLoc location, List* children);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

List* getCallTokenChildren(CallToken* token);

class BinaryOpToken : public Token {
public:
    const char* op;
    Token* left;
    Token* right;

    BinaryOpToken(SrcLoc location, const char* op, Token* left, Token* right);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

const char* getBinaryOpTokenOp(BinaryOpToken* token);
Token* getBinaryOpTokenLeft(BinaryOpToken* token);
Token* getBinaryOpTokenRight(BinaryOpToken* token);

class UnaryOpToken : public Token {
public:
    const char* op;
    Token* child;

    UnaryOpToken(SrcLoc location, const char* op, Token* child) ;
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

const char* getUnaryOpTokenOp(UnaryOpToken* token);
Token* getUnaryOpTokenChild(UnaryOpToken* token);

class AssignmentToken : public Token {
public:
    Token* left;
    Token* right;

    AssignmentToken(SrcLoc location, Token* left, Token* right);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

Token* getAssignmentTokenLeft(AssignmentToken* token);
Token* getAssignmentTokenRight(AssignmentToken* token);

class BlockToken : public Token {
public:
    List* children;

    BlockToken(SrcLoc location, List* children);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

List* getBlockTokenChildren(BlockToken* token);

typedef struct {
    Token* condition;
    BlockToken* block;
} IfBranch;

class IfToken : public Token {
public:
    List* branches;
    BlockToken* elseBranch;

    IfToken(SrcLoc location, List* branches, BlockToken* elseBranch);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

List* getIfTokenBranches(IfToken* token);
BlockToken* getIfTokenElseBranch(IfToken* token);

class WhileToken : public Token {
public:
    Token* condition;
    BlockToken* body;

    WhileToken(SrcLoc location, Token* condition, BlockToken* body);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

Token* getWhileTokenCondition(WhileToken* token);
BlockToken* getWhileTokenBody(WhileToken* token);

class FuncToken : public Token {
public:
    IdentifierToken* name;
    List* args;
    BlockToken* body;

    FuncToken(SrcLoc loc, IdentifierToken* name, List* args, BlockToken* body);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

IdentifierToken* getFuncTokenName(FuncToken* token);
void setFuncTokenName(FuncToken* token, IdentifierToken* name);
List* getFuncTokenArgs(FuncToken* token);
BlockToken* getFuncTokenBody(FuncToken* token);

class ReturnToken : public Token {
public:
    Token* body;

    ReturnToken(SrcLoc location, Token* body);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

Token* getReturnTokenBody(ReturnToken* token);

class LabelToken : public Token {
public:
    const char* name;

    LabelToken(SrcLoc location, const char* name);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

const char* getLabelTokenName(LabelToken* token);

class AbsJumpToken : public Token {
public:
    const char* label;

    AbsJumpToken(SrcLoc location, const char* label);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

const char* getAbsJumpTokenLabel(AbsJumpToken* token);

class CondJumpToken : public Token {
public:
    Token* condition;
    const char* label;
    //if the condition is false and when is 0 then the jump is taken
    //if the condition is true and when is 1 then the jump is taken
    uint8_t when;

    CondJumpToken(SrcLoc loc, Token* condition, const char* label, uint8_t when);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

Token* getCondJumpTokenCondition(CondJumpToken* token);
const char* getCondJumpTokenLabel(CondJumpToken* token);
uint8_t getCondJumpTokenWhen(CondJumpToken* token);

class PushBuiltinToken : public Token {
public:
    const char* name;

    PushBuiltinToken(SrcLoc location, const char* name);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

const char* getPushBuiltinTokenName(PushBuiltinToken* token);

class PushIntToken : public Token {
public:
    int32_t value;

    PushIntToken(SrcLoc location, int32_t value);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};
int32_t getPushIntTokenValue(PushIntToken* token);

class CallOpToken : public Token {
public:
    uint8_t arity;

    CallOpToken(SrcLoc location, uint8_t arity);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

uint8_t getCallOpTokenArity(CallOpToken* token);

class StoreToken : public Token {
public:
    const char* name;

    StoreToken(SrcLoc location, const char* name);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

const char* getStoreTokenName(StoreToken* token);

class DupToken : public Token {
public:
    DupToken(SrcLoc location);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

class PushToken : public Token {
public:
    Token* value;

    PushToken(SrcLoc location, Token* value);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t ident);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

Token* getPushTokenValue(PushToken* token);

class Rot3Token : public Token {
public:
    Rot3Token(SrcLoc location);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

class SwapToken : public Token {
public:
    SwapToken(SrcLoc location);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

class PopToken : public Token {
public:
    PopToken(SrcLoc location);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

class BuiltinToken : public Token {
public:
    const char* name;

    BuiltinToken(SrcLoc location, const char* name);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

const char* getBuiltinTokenName(BuiltinToken* token);

class CheckNoneToken : public Token {
public:
    CheckNoneToken(SrcLoc location);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

class NewFuncToken : public Token {
public:
    const char* name;

    NewFuncToken(SrcLoc location, const char* name);
    TokenType type();
    void destroy(Token* self);
    void print(Token* self, uint8_t indent);
    uint8_t equals(Token* self, Token* other);
    Token* copy(Token* self, CopyVisitor visitor, void* data);
};

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
