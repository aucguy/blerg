#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "main/tokens.h"
#include "main/util.h"

#define UNUSED(x) (void)(x)
#define tokenInstanceFields {0, 0}

Token createLegacyTokenType(LegacyTokenInit init) {
    LegacyTokenMethods* methods = new LegacyTokenMethods();
    methods->typeValue = init.type;
    methods->destroyFunc = init.destroy;
    methods->printFunc = init.print;
    methods->equalsFunc = init.equals;
    methods->copyFunc = init.copy;

    Token token;
    token.methods = methods;
    token.location_.line = 0;
    token.location_.column = 0;

    return token;
}

Token createTokenType(TokenMethods* methods) {
    Token token;
    token.methods = methods;
    token.location_.line = 0;
    token.location_.column = 0;
    return token;
}


LegacyTokenMethods::LegacyTokenMethods() :
        typeValue(TOKEN_UNDEF), destroyFunc(nullptr), printFunc(nullptr),
        equalsFunc(nullptr), copyFunc(nullptr) {}

LegacyTokenMethods::~LegacyTokenMethods() {}

TokenType LegacyTokenMethods::type() {
    return this->typeValue;
}

void LegacyTokenMethods::destroy(Token* self) {}

void LegacyTokenMethods::print(Token* token, uint8_t indent) {
    this->printFunc(token, indent);
}

uint8_t LegacyTokenMethods::equals(Token* self, Token* other) {
    return this->equalsFunc(self, other);
}

Token* LegacyTokenMethods::copy(Token* self, CopyVisitor visitor, void* data) {
    return this->copyFunc(self, visitor, data);
}

uint8_t tokensEqualVoid(void* a, void* b);
uint8_t branchesEqual(void* a, void* b);

SrcLoc tokenLocation(Token* token) {
    return token->location_;
}

void setTokenLocation(Token* token, SrcLoc loc) {
    token->location_ = loc;
}

TokenType getTokenType(Token* token) {
    return token->methods->type();
}

void setTokenType(Token* token, Token type) {
    *token = type;
}

void printIndent(uint8_t indent) {
    for(uint8_t i = 0; i < indent; i++) {
        printf("    ");
    }
}

List* copyTokenList(List* old, CopyVisitor visitor, void* data) {
    if(old == NULL) {
        return NULL;
    } else {
        Token* head = visitor((Token*) old->head, data);
        List* tail = copyTokenList(old->tail, visitor, data);
        return consList(head, tail);
    }
}

class IntTokenMethods : public TokenMethods {
public:
    int32_t value;

    IntTokenMethods(int32_t value) :
        value(value) {}

    TokenType type() {
        return TOKEN_INT;
    }

    void destroy(Token* self) {
        UNUSED(self);
    }

    void print(Token* self, uint8_t indent) {
        UNUSED(indent);
        printf("int: %i\n", getIntTokenValue((IntToken*) self));
    }

    uint8_t equals(Token* self, Token* other) {
        return getIntTokenValue((IntToken*) self) == getIntTokenValue((IntToken*) other);
    }

    Token* copy(Token* token, CopyVisitor visitor, void* data) {
        UNUSED(visitor);
        UNUSED(data);
        IntToken* intToken = (IntToken*) token;
        return (Token*) createIntToken(tokenLocation((Token*) intToken), getIntTokenValue(intToken));
    }
};

int32_t getIntTokenValue(IntToken* token) {
    return ((IntTokenMethods*) token->token_.methods)->value;
}

/**
 * constructs a integer token
 *
 * @param value the token's value
 * @return the newly created token
 */
IntToken* createIntToken(SrcLoc loc, int32_t value) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    setTokenType(&token->token_, createTokenType(new IntTokenMethods(value)));
    setTokenLocation(&token->token_, loc);
    return token;
}

class FloatTokenMethods : public TokenMethods {
public:
    float value;

    FloatTokenMethods(float value) :
        value(value) {}

    TokenType type() {
        return TOKEN_FLOAT;
    }

    void destroy(Token* self) {
        UNUSED(self);
    }

    void print(Token* self, uint8_t indent) {
        UNUSED(indent);
        printf("float: %f\n", getFloatTokenValue((FloatToken*) self));
    }

    uint8_t equals(Token* self, Token* other) {
        return getFloatTokenValue(((FloatToken*) self)) == getFloatTokenValue(((FloatToken*) other));
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        UNUSED(visitor);
        UNUSED(data);
        FloatToken* num = (FloatToken*) self;
        return (Token*) createFloatToken(tokenLocation(self), getFloatTokenValue(num));
    }
};

float getFloatTokenValue(FloatToken* token) {
    return ((FloatTokenMethods*) token->token_.methods)->value;
}

FloatToken* createFloatToken(SrcLoc location, float value) {
    FloatToken* token = (FloatToken*) malloc(sizeof(FloatToken));
    setTokenType(&token->token_, createTokenType(new FloatTokenMethods(value)));
    setTokenLocation(&token->token_, location);
    return token;
}

class LiteralTokenMethods : public TokenMethods {
public:
    const char* value;

    LiteralTokenMethods(const char* value) :
        value(value) {}

    TokenType type() {
        return TOKEN_LITERAL;
    }

    void destroy(Token* self) {
        //free((void*) ((LiteralToken*) self)->value);
    }

    void print(Token* self, uint8_t indent) {
        UNUSED(indent);
        printf("literal: %s\n", getLiteralTokenValue((LiteralToken*) self));
    }

    uint8_t equals(Token* self, Token* other) {
        return strcmp(getLiteralTokenValue((LiteralToken*) self),
                getLiteralTokenValue((LiteralToken*) other)) == 0;
    }

    Token* copy(Token* token, CopyVisitor visitor, void* data) {
        UNUSED(visitor);
        UNUSED(data);
        LiteralToken* literal = (LiteralToken*) token;
        const char* copied = newStr(getLiteralTokenValue(literal));
        return (Token*) createLiteralToken(tokenLocation((Token*) literal), copied);
    }
};

const char* getLiteralTokenValue(LiteralToken* token) {
    return ((LiteralTokenMethods*) token->token_.methods)->value;
}

/**
 * constructs a literal token
 *
 * @param value a unique reference to the token's value.
 * @return the newly created token
 */
LiteralToken* createLiteralToken(SrcLoc loc, const char* value) {
    LiteralToken* token = (LiteralToken*) malloc(sizeof(LiteralToken));
    setTokenType(&token->token_, createTokenType(new LiteralTokenMethods(value)));
    setTokenLocation(&token->token_, loc);
    return token;
}

class IdentifierTokenMethods : public TokenMethods {
public:
    const char* value;

    IdentifierTokenMethods(const char* value) :
        value(value) {}

    TokenType type() {
        return TOKEN_IDENTIFIER;
    }

    void destroy(Token* token) {
        //free((void*) ((IdentifierToken*) token)->value);
    }

    void print(Token* self, uint8_t indent) {
        UNUSED(indent);
        printf("identifier: %s\n", (getIdentifierTokenValue((IdentifierToken*) self)));
    }

    uint8_t equals(Token* self, Token* other) {
        return strcmp(getIdentifierTokenValue((IdentifierToken*) self),
                getIdentifierTokenValue((IdentifierToken*) other)) == 0;
    }

    Token* copy(Token* token, CopyVisitor visitor, void* data) {
        UNUSED(visitor);
        UNUSED(data);
        IdentifierToken* identifier = (IdentifierToken*) token;
        SrcLoc loc = tokenLocation((Token*) identifier);
        return (Token*) createIdentifierToken(loc, newStr(getIdentifierTokenValue(identifier)));
    }
};

const char* getIdentifierTokenValue(IdentifierToken* token) {
    return ((IdentifierTokenMethods*) token->token_.methods)->value;
}

/**
 * constructs an identifier token
 *
 * @param value a unique reference to the token's name.
 * @return the newly created token
 */
IdentifierToken* createIdentifierToken(SrcLoc loc, const char* value) {
    IdentifierToken* token = (IdentifierToken*) malloc(sizeof(IdentifierToken));
    setTokenType(&token->token_, createTokenType(new IdentifierTokenMethods(value)));
    setTokenLocation(&token->token_, loc);
    return token;
}

class TupleTokenMethods : public TokenMethods {
public:
    List* elements;

    TupleTokenMethods(List* elements) :
        elements(elements) {}

    TokenType type() {
        return TOKEN_TUPLE;
    }

    void destroy(Token* self) {
        //TupleToken* tuple = (TupleToken*) self;
        //destroyList(tuple->elements, destroyTokenVoid);
    }

    void print(Token* self, uint8_t indent) {
        printf("tuple:\n");
        List* elements = getTupleTokenElements((TupleToken*) self);
        uint8_t i = 0;

        while(elements != NULL) {
            printIndent(indent + 1);
            printf("%i:\n", i);
            printTokenWithIndent((Token*) elements->head, indent + 2);
            elements = elements->tail;
            i++;
        }
    }

    uint8_t equals(Token* self, Token* other) {
        TupleToken* tuple1 = (TupleToken*) self;
        TupleToken* tuple2 = (TupleToken*) other;
        return allList2(getTupleTokenElements(tuple1), getTupleTokenElements(tuple2), tokensEqualVoid);
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        TupleToken* tuple = (TupleToken*) self;
        List* copied = copyTokenList(getTupleTokenElements(tuple), visitor, data);
        return (Token*) createTupleToken(tokenLocation((Token*) tuple), copied);
    }
};

List* getTupleTokenElements(TupleToken* token) {
    return ((TupleTokenMethods*) token->token_.methods)->elements;
}

TupleToken* createTupleToken(SrcLoc location, List* elements) {
    TupleToken* tuple = (TupleToken*) malloc(sizeof(TupleToken));
    setTokenType(&tuple->token_, createTokenType(new TupleTokenMethods(elements)));
    setTokenLocation(&tuple->token_, location);
    return tuple;
}

class ListTokenMethods : public TokenMethods {
public:
    List* elements;

    ListTokenMethods(List* elements) :
        elements(elements) {}

    TokenType type() {
        return TOKEN_LIST;
    }

    void destroy(Token* self) {
        //ListToken* list = (ListToken*) self;
        //destroyList(list->elements, destroyTokenVoid);
    }

    void print(Token* self, uint8_t indent) {
        printf("list:\n");
        List* elements = getListTokenElements((ListToken*) self);
        uint8_t i = 0;

        while(elements != NULL) {
            //TODO remove this line to fix formating
            printIndent(indent + 1);
            printf("%i:\n", i);
            printTokenWithIndent((Token*) elements->head, indent + 2);
            elements = elements->tail;
            i++;
        }
    }

    uint8_t equals(Token* self, Token* other) {
        ListToken* list1 = (ListToken*) self;
        ListToken* list2 = (ListToken*) other;
        return allList2(getListTokenElements(list1), getListTokenElements(list2),
                tokensEqualVoid);
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        ListToken* list = (ListToken*) self;
        List* copied = copyTokenList(getListTokenElements(list), visitor, data);
        return (Token*) createListToken(tokenLocation((Token*) list), copied);
    }
};

List* getListTokenElements(ListToken* token) {
    return ((ListTokenMethods*) token->token_.methods)->elements;
}

ListToken* createListToken(SrcLoc location, List* elements) {
    ListToken* list = (ListToken*) malloc(sizeof(ListToken));
    setTokenType(&list->token_, createTokenType(new ListTokenMethods(elements)));
    setTokenLocation(&list->token_, location);
    return list;
}

ObjectPair* createObjectPair(Token* key, Token* value) {
    ObjectPair* pair = (ObjectPair*) malloc(sizeof(ObjectPair));
    pair->key = key;
    pair->value = value;
    return pair;
}

List* copyObjectPairs(List* list, CopyVisitor visitor, void* data) {
    if(list == NULL) {
        return NULL;
    } else {
        ObjectPair* pair = (ObjectPair*) list->head;
        Token* key = visitor(pair->key, data);
        Token* value = visitor(pair->value, data);
        ObjectPair* head = createObjectPair(key, value);
        List* tail = copyObjectPairs(list->tail, visitor, data);
        return consList(head, tail);
    }
}

class ObjectTokenMethods : public TokenMethods {
public:
    List* elements;

    ObjectTokenMethods(List* elements) :
        elements(elements) {}

    TokenType type() {
        return TOKEN_OBJECT;
    }

    void destroy(Token* self) {
        ObjectToken* object = (ObjectToken*) self;
        List* elements = getObjectTokenElements(object);

        while(elements != NULL) {
            ObjectPair* pair = (ObjectPair*) elements->head;
            destroyToken(pair->key);
            destroyToken(pair->value);
            free(pair);
            elements = elements->tail;
        }
        destroyShallowList(getObjectTokenElements(object));
    }

    void print(Token* self, uint8_t indent) {
        ObjectToken* object = (ObjectToken*) self;
        printf("object:\n");

        List* elements = getObjectTokenElements(object);
        uint8_t count = 0;
        while(elements != NULL) {
            ObjectPair* pair = (ObjectPair*) elements->head;

            printIndent(indent + 1);
            printf("%i:\n", count);

            printIndent(indent + 2);
            printf("key:\n");
            printTokenWithIndent(pair->key, indent + 3);

            printIndent(indent + 2);
            printf("value:\n");
            printTokenWithIndent(pair->value, indent + 3);

            count++;
            elements = elements->tail;
        }
    }

    uint8_t equals(Token* self, Token* other) {
        ObjectToken* object1 = (ObjectToken*) self;
        ObjectToken* object2 = (ObjectToken*) other;

        List* elements1 = getObjectTokenElements(object1);
        List* elements2 = getObjectTokenElements(object2);

        while(elements1 != NULL && elements2 != NULL) {
            ObjectPair* pair1 = (ObjectPair*) elements1->head;
            ObjectPair* pair2 = (ObjectPair*) elements2->head;
            if(!tokensEqual(pair1->key, pair2->key) ||
                    !tokensEqual(pair1->value, pair2->value)) {
                return 0;
            }
            elements1 = elements1->tail;
            elements2 = elements2->tail;
        }

        return elements1 == NULL && elements2 == NULL;
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        ObjectToken* object = (ObjectToken*) self;
        List* copied = copyObjectPairs(getObjectTokenElements(object), visitor, data);
        return (Token*) createObjectToken(tokenLocation(self), copied);
    }
};

List* getObjectTokenElements(ObjectToken* token) {
    return ((ObjectTokenMethods*) token->token_.methods)->elements;
}

ObjectToken* createObjectToken(SrcLoc location, List* elements) {
    ObjectToken* object = (ObjectToken*) malloc(sizeof(ObjectToken));
    setTokenType(&object->token_, createTokenType(new ObjectTokenMethods(elements)));
    setTokenLocation(&object->token_, location);
    return object;
}

class CallTokenMethods : public TokenMethods {
public:
    List* children;

    CallTokenMethods(List* children) :
        children(children) {}

    TokenType type() {
        return TOKEN_CALL;
    }

    void destroy(Token* self) {
        List* children = getCallTokenChildren((CallToken*) self);
        while(children != NULL) {
            destroyToken((Token*) children->head);
            children = children->tail;
        }
        destroyShallowList(getCallTokenChildren((CallToken*) self));
    }

    void print(Token* self, uint8_t indent) {
        printf("call:\n");
        List* children = getCallTokenChildren((CallToken*) self);
        while(children != NULL) {
            printTokenWithIndent((Token*) children->head, indent + 1);
            children = children->tail;
        }
    }

    uint8_t equals(Token* self, Token* other) {
        List* childrenA = getCallTokenChildren((CallToken*) self);
        List* childrenB = getCallTokenChildren((CallToken*) other);

        while(childrenA != NULL && childrenB != NULL) {
            if(!tokensEqual((Token*) childrenA->head, (Token*) childrenB->head)) {
                return 0;
            }
            childrenA = childrenA->tail;
            childrenB = childrenB->tail;
        }

        return childrenA == NULL && childrenB == NULL;
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        CallToken* call = (CallToken*) self;
        List* children = copyTokenList(getCallTokenChildren(call), visitor, data);
        return (Token*) createCallToken(tokenLocation((Token*) call), children);
    }
};

List* getCallTokenChildren(CallToken* token) {
    return ((CallTokenMethods*) token->token_.methods)->children;
}

CallToken* createCallToken(SrcLoc location, List* children) {
    CallToken* token = (CallToken*) malloc(sizeof(CallToken));
    setTokenType(&token->token_, createTokenType(new CallTokenMethods(children)));
    setTokenLocation(&token->token_, location);
    return token;
}

class BinaryOpTokenMethods : public TokenMethods {
public:
    const char* op;
    Token* left;
    Token* right;

    BinaryOpTokenMethods(const char* op, Token* left, Token* right) :
        op(op), left(left), right(right) {}

    TokenType type() {
        return TOKEN_BINARY_OP;
    }

    void destroy(Token* self) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) self;
        free((void*) getBinaryOpTokenOp(binaryOp));
        destroyToken(getBinaryOpTokenLeft(binaryOp));
        destroyToken(getBinaryOpTokenRight(binaryOp));
    }

    void print(Token* self, uint8_t indent) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) self;
        printf("binaryOp: '%s'\n", getBinaryOpTokenOp(binaryOp));
        printTokenWithIndent(getBinaryOpTokenLeft(binaryOp), indent + 1);
        printTokenWithIndent(getBinaryOpTokenRight(binaryOp), indent + 1);
    }

    uint8_t equals(Token* self, Token* other) {
        BinaryOpToken* selfBinOp = (BinaryOpToken*) self;
        BinaryOpToken* otherBinOp = (BinaryOpToken*) other;
        return strcmp(getBinaryOpTokenOp(selfBinOp), getBinaryOpTokenOp(otherBinOp)) == 0 &&
                tokensEqual(getBinaryOpTokenLeft(selfBinOp), getBinaryOpTokenLeft(otherBinOp)) &&
                tokensEqual(getBinaryOpTokenRight(selfBinOp), getBinaryOpTokenRight(otherBinOp));
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        BinaryOpToken* binOp = (BinaryOpToken*) self;
        const char* op = newStr(getBinaryOpTokenOp(binOp));
        Token* left = visitor(getBinaryOpTokenLeft(binOp), data);
        Token* right = visitor(getBinaryOpTokenRight(binOp), data);
        return (Token*) createBinaryOpToken(tokenLocation((Token*) binOp), op, left, right);
    }
};

const char* getBinaryOpTokenOp(BinaryOpToken* token) {
    return ((BinaryOpTokenMethods*) token->token_.methods)->op;
}

Token* getBinaryOpTokenLeft(BinaryOpToken* token) {
    return ((BinaryOpTokenMethods*) token->token_.methods)->left;
}

Token* getBinaryOpTokenRight(BinaryOpToken* token) {
    return ((BinaryOpTokenMethods*) token->token_.methods)->right;
}

/**
 * constructs a binary operation token
 *
 * @param op a unique reference to the token's operation.
 * @param left a unique reference to the left operand.
 * @param right a unique reference to the right operand.
 * @return the newly created token
 */
BinaryOpToken* createBinaryOpToken(SrcLoc loc, const char* op, Token* left,
        Token* right) {
    BinaryOpToken* token = (BinaryOpToken*) malloc(sizeof(BinaryOpToken));
    setTokenType(&token->token_,
            createTokenType(new BinaryOpTokenMethods(op, left, right)));
    setTokenLocation(&token->token_, loc);
    return token;
}

class UnaryOpTokenMethods : public TokenMethods {
public:
    const char* op;
    Token* child;

    UnaryOpTokenMethods(const char* op, Token* child) :
        op(op), child(child) {}

    TokenType type() {
        return TOKEN_UNARY_OP;
    }

    void destroy(Token* self) {
        UnaryOpToken* unaryOp = (UnaryOpToken*) self;
        free((void*) getUnaryOpTokenOp(unaryOp));
        destroyToken(getUnaryOpTokenChild(unaryOp));
    }

    void print(Token* self, uint8_t indent) {
        UnaryOpToken* unaryOp = (UnaryOpToken*) self;
        printf("unaryOp: %s\n", getUnaryOpTokenOp(unaryOp));
        printTokenWithIndent(getUnaryOpTokenChild(unaryOp), indent + 1);
    }

    uint8_t equals(Token* self, Token* other) {
        UnaryOpToken* selfUnOp = (UnaryOpToken*) self;
        UnaryOpToken* otherUnOp = (UnaryOpToken*) other;
        return strcmp(getUnaryOpTokenOp(selfUnOp), getUnaryOpTokenOp(otherUnOp)) == 0 &&
                tokensEqual(getUnaryOpTokenChild(selfUnOp), getUnaryOpTokenChild(otherUnOp));
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        UnaryOpToken* unOp = (UnaryOpToken*) self;
        return (Token*) createUnaryOpToken(tokenLocation(self),
                newStr(getUnaryOpTokenOp(unOp)),
                visitor(getUnaryOpTokenChild(unOp), data));
    }
};

const char* getUnaryOpTokenOp(UnaryOpToken* token) {
    return ((UnaryOpTokenMethods*) token->token_.methods)->op;
}

Token* getUnaryOpTokenChild(UnaryOpToken* token) {
    return ((UnaryOpTokenMethods*) token->token_.methods)->child;
}

/**
 * constructs a unary op token
 *
 * @param op a unique reference to the token's operation.
 * @param child a unique reference to the token's operand.
 * @return the newly created token
 */
UnaryOpToken* createUnaryOpToken(SrcLoc loc, const char* op, Token* child) {
    UnaryOpToken* token = (UnaryOpToken*) malloc(sizeof(UnaryOpToken));
    setTokenType(&token->token_, createTokenType(new UnaryOpTokenMethods(op, child)));
    setTokenLocation(&token->token_, loc);
    return token;
}

class AssignmentTokenMethods : public TokenMethods {
public:
    Token* left;
    Token* right;

    AssignmentTokenMethods(Token* left, Token* right) :
        left(left), right(right) {}

    TokenType type() {
        return TOKEN_ASSIGNMENT;
    }

    void destroy(Token* self) {
        //AssignmentToken* assignment = (AssignmentToken*) self;
        //destroyToken((Token*) assignment->left);
        //destroyToken(assignment->right);
    }

    void print(Token* self, uint8_t indent) {
        AssignmentToken* assignment = (AssignmentToken*) self;
        printf("assignment:\n");

        printIndent(indent + 1);
        printf("lvalue:\n");
        printTokenWithIndent(getAssignmentTokenLeft(assignment), indent + 2);

        printIndent(indent + 1);
        printf("rvalue:\n");
        printTokenWithIndent(getAssignmentTokenRight(assignment), indent + 2);
    }

    uint8_t equals(Token* self, Token* other) {
        AssignmentToken* selfAssign = (AssignmentToken*) self;
        AssignmentToken* otherAssign = (AssignmentToken*) other;

        Token* selfLeft = getAssignmentTokenLeft(selfAssign);
        Token* selfRight = getAssignmentTokenRight(otherAssign);

        Token* otherLeft = getAssignmentTokenLeft(otherAssign);
        Token* otherRight = getAssignmentTokenRight(otherAssign);

        return tokensEqual(selfLeft, otherLeft) &&
                tokensEqual(selfRight, otherRight);
    }

    Token* copy(Token* self, CopyVisitor visitor, void* data) {
        AssignmentToken* assign = (AssignmentToken*) self;
        Token* left = visitor((Token*) getAssignmentTokenLeft(assign), data);
        Token* right = visitor(getAssignmentTokenRight(assign), data);
        return (Token*) createAssignmentToken(tokenLocation(self), left, right);
    }
};

Token* getAssignmentTokenLeft(AssignmentToken* token) {
    return ((AssignmentTokenMethods*) token->token_.methods)->left;
}

Token* getAssignmentTokenRight(AssignmentToken* token) {
    return ((AssignmentTokenMethods*) token->token_.methods)->right;
}

/**
 * constructs an assignment token
 *
 * @param left a unique reference to the token's lvalue.
 * @param right a unique reference to the token's rvalue.
 */
AssignmentToken* createAssignmentToken(SrcLoc loc, Token* left, Token* right) {
    AssignmentToken* token = (AssignmentToken*) malloc(sizeof(AssignmentToken));
    setTokenType(&token->token_,
            createTokenType(new AssignmentTokenMethods(left, right)));
    setTokenLocation(&token->token_, loc);
    return token;
}

void destroyBlockToken(Token* self) {
    destroyList(((BlockToken*) self)->children, destroyTokenVoid);
}

void printBlockToken(Token* self, uint8_t indent) {
    BlockToken* block = (BlockToken*) self;
    printf("block:\n");
    for(List* node = block->children; node != NULL; node = node->tail) {
        printTokenWithIndent((Token*) node->head, indent + 1);
    }
}

uint8_t equalsBlockToken(Token* self, Token* other) {
    return allList2(((BlockToken*) self)->children,
            ((BlockToken*) other)->children, tokensEqualVoid);
}

Token* copyBlockToken(Token* self, CopyVisitor visitor, void* data) {
    BlockToken* block = (BlockToken*) self;
    List* copied = copyTokenList(block->children, visitor, data);
    SrcLoc location = tokenLocation(self);
    return (Token*) createBlockToken(location, copied);
}

LegacyTokenInit BLOCK_TYPE_INIT = {
        TOKEN_BLOCK,
        destroyBlockToken,
        printBlockToken,
        equalsBlockToken,
        copyBlockToken
};

Token BLOCK_TYPE = createLegacyTokenType(BLOCK_TYPE_INIT);

BlockToken* createBlockToken(SrcLoc location, List* children) {
    BlockToken* token = (BlockToken*) malloc(sizeof(BlockToken));
    setTokenType(&token->token_, BLOCK_TYPE);
    setTokenLocation(&token->token_, location);
    token->children = children;
    return token;
}

IfBranch* createIfBranch(Token* condition, BlockToken* block) {
    IfBranch* branch = (IfBranch*) malloc(sizeof(IfBranch));
    branch->condition = condition;
    branch->block = block;
    return branch;
}

void destroyIfToken(Token* self) {
    IfToken* ifToken = (IfToken*) self;
    destroyList(ifToken->branches, destroyIfBranch);
    destroyToken((Token*) ifToken->elseBranch);
}

void printIfToken(Token* self, uint8_t indent) {
    IfToken* ifStmt = (IfToken*) self;
    printf("if:\n");
    for(List* node = ifStmt->branches; node != NULL; node = node->tail) {
        IfBranch* branch = (IfBranch*) node->head;
        printIndent(indent + 1);
        printf("condition:\n");
        printTokenWithIndent((Token*) branch->condition, indent + 2);
        printIndent(indent + 1);
        printf("body:\n");
        printTokenWithIndent((Token*) branch->block, indent + 2);
    }
    printIndent(indent + 1);
    printf("else:\n");
    printTokenWithIndent((Token*) ifStmt->elseBranch, indent + 2);
}

uint8_t equalsIfToken(Token* self, Token* other) {
    IfToken* selfIf = (IfToken*) self;
    IfToken* otherIf = (IfToken*) other;

    if(!allList2(selfIf->branches, otherIf->branches, branchesEqual)) {
        return 0;
    }
    if(selfIf->elseBranch == NULL && otherIf->elseBranch == NULL) {
        return 1;
    }
    return tokensEqual((Token*) selfIf->elseBranch, (Token*) otherIf->elseBranch);
}

List* copyIfBranches(List* branches, CopyVisitor visitor, void* data) {
    if(branches == NULL) {
        return NULL;
    } else {
        IfBranch* branch = (IfBranch*) branches->head;
        Token* condition = visitor(branch->condition, data);
        BlockToken* block = (BlockToken*) visitor((Token*) branch->block, data);
        IfBranch* head = createIfBranch(condition, block);
        List* tail = copyIfBranches(branches->tail, visitor, data);
        return consList(head, tail);
    }
}

Token* copyIfToken(Token* self, CopyVisitor visitor, void* data) {
    IfToken* ifToken = (IfToken*) self;
    List* branches = copyIfBranches(ifToken->branches, visitor, data);
    Token* elseBranch;
    if(ifToken->elseBranch == NULL) {
        elseBranch = NULL;
    } else {
        elseBranch = visitor((Token*) ifToken->elseBranch, data);
    }
    return (Token*) createIfToken(tokenLocation(self), branches,
            (BlockToken*) elseBranch);
}

LegacyTokenInit IF_TYPE_INIT = {
        TOKEN_IF,
        destroyIfToken,
        printIfToken,
        equalsIfToken,
        copyIfToken
};

Token IF_TYPE = createLegacyTokenType(IF_TYPE_INIT);

IfToken* createIfToken(SrcLoc loc, List* branches, BlockToken* elseBranch) {
    IfToken* token = (IfToken*) malloc(sizeof(IfToken));
    setTokenType(&token->token_, IF_TYPE);
    setTokenLocation(&token->token_, loc);
    token->branches = branches;
    token->elseBranch = elseBranch;
    return token;
}

void destroyWhileToken(Token* self) {
    WhileToken* whileToken = (WhileToken*) self;
    destroyToken((Token*) whileToken->condition);
    destroyToken((Token*) whileToken->body);
}

void printWhileToken(Token* self, uint8_t indent) {
    WhileToken* whileStmt = (WhileToken*) self;
    printf("while:\n");

    printIndent(indent + 1);
    printf("condition:\n");
    printTokenWithIndent((Token*) whileStmt->condition, indent + 2);

    printIndent(indent + 1);
    printf("body:\n");
    printTokenWithIndent((Token*) whileStmt->body, indent + 2);
}

uint8_t equalsWhileToken(Token* self, Token* other) {
    WhileToken* selfWhile = (WhileToken*) self;
    WhileToken* otherWhile = (WhileToken*) other;
    return tokensEqual((Token*) selfWhile->condition, (Token*) otherWhile->condition) &&
            tokensEqual((Token*) selfWhile->body, (Token*) otherWhile->body);
}

Token* copyWhileToken(Token* self, CopyVisitor visitor, void* data) {
    WhileToken* whileToken = (WhileToken*) self;
    Token* condition = visitor(whileToken->condition, data);
    BlockToken* body = (BlockToken*) visitor((Token*) whileToken->body, data);
    return (Token*) createWhileToken(tokenLocation(self), condition, body);
}

LegacyTokenInit WHILE_TYPE_INIT = {
        TOKEN_WHILE,
        destroyWhileToken,
        printWhileToken,
        equalsWhileToken,
        copyWhileToken
};

Token WHILE_TYPE = createLegacyTokenType(WHILE_TYPE_INIT);

WhileToken* createWhileToken(SrcLoc loc, Token* condition, BlockToken* body) {
    WhileToken* token = (WhileToken*) malloc(sizeof(WhileToken));
    setTokenType(&token->token_, WHILE_TYPE);
    setTokenLocation(&token->token_, loc);
    token->condition = condition;
    token->body = body;
    return token;
}

void destroyFuncToken(Token* self) {
    FuncToken* funcToken = (FuncToken*) self;
    destroyToken((Token*) funcToken->name);
    destroyList(funcToken->args, destroyTokenVoid);
    destroyToken((Token*) funcToken->body);
}

void printFuncToken(Token* self, uint8_t indent) {
    FuncToken* func = (FuncToken*) self;
    printf("func: %s", getIdentifierTokenValue(func->name));
    List* node = func->args;
    while(node != NULL) {
        printf(" %s", getIdentifierTokenValue((IdentifierToken*) node->head));
        node = node->tail;
    }
    printf("\n");
    printTokenWithIndent((Token*) func->body, indent + 1);
}

uint8_t equalsFuncToken(Token* self, Token* other) {
    FuncToken* selfFunc = (FuncToken*) self;
    FuncToken* otherFunc = (FuncToken*) other;
    return tokensEqual((Token*) selfFunc->name, (Token*) otherFunc->name) &&
            allList2(selfFunc->args, otherFunc->args, tokensEqualVoid) &&
            tokensEqual((Token*) selfFunc->body, (Token*) otherFunc->body);
}

Token* copyFuncToken(Token* self, CopyVisitor visitor, void* data) {
    FuncToken* func = (FuncToken*) self;
    IdentifierToken* name = (IdentifierToken*) visitor((Token*) func->name, data);
    List* args = copyTokenList(func->args, visitor, data);
    BlockToken* body = (BlockToken*) visitor((Token*) func->body, data);

    return (Token*) createFuncToken(tokenLocation(self), name, args, body);
}

LegacyTokenInit FUNC_TYPE_INIT = {
        TOKEN_FUNC,
        destroyFuncToken,
        printFuncToken,
        equalsFuncToken,
        copyFuncToken
};

Token FUNC_TYPE = createLegacyTokenType(FUNC_TYPE_INIT);

FuncToken* createFuncToken(SrcLoc loc, IdentifierToken* name, List* args,
        BlockToken* body) {
    FuncToken* token = (FuncToken*) malloc(sizeof(FuncToken));
    setTokenType(&token->token_, FUNC_TYPE);
    setTokenLocation(&token->token_, loc);
    token->name = name;
    token->args = args;
    token->body = body;
    return token;
}

void destroyReturnToken(Token* self) {
    destroyToken(((ReturnToken*) self)->body);
}

void printReturnToken(Token* self, uint8_t indent) {
    printf("return:\n");
    printTokenWithIndent(((ReturnToken*) self)->body, indent + 1);
}

uint8_t equalsReturnToken(Token* self, Token* other) {
    return tokensEqual(((ReturnToken*) self)->body, ((ReturnToken*) other)->body);
}

Token* copyReturnToken(Token* self, CopyVisitor visitor, void* data) {
    ReturnToken* token = (ReturnToken*) self;
    Token* copied = visitor(token->body, data);
    return (Token*) createReturnToken(tokenLocation(self), copied);
}

LegacyTokenInit RETURN_TYPE_INIT = {
        TOKEN_RETURN,
        destroyReturnToken,
        printReturnToken,
        equalsReturnToken,
        copyReturnToken
};

Token RETURN_TYPE = createLegacyTokenType(RETURN_TYPE_INIT);

ReturnToken* createReturnToken(SrcLoc location, Token* body) {
    ReturnToken* token = (ReturnToken*) malloc(sizeof(ReturnToken));
    setTokenType(&token->token_, RETURN_TYPE);
    setTokenLocation(&token->token_, location);
    token->body = body;
    return token;
}

void destroyLabelToken(Token* self) {
    free((void*) ((LabelToken*) self)->name);
}

void printLabelToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("label: %s \n", ((LabelToken*) self)->name);
}

uint8_t equalsLabelToken(Token* self, Token* other) {
    return strcmp(((LabelToken*) self)->name, ((LabelToken*) other)->name) == 0;
}

Token* copyLabelToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    LabelToken* label = (LabelToken*) self;
    const char* name = newStr(label->name);
    return (Token*) createLabelToken(name);
}

LegacyTokenInit LABEL_TYPE_INIT = {
        TOKEN_LABEL,
        destroyLabelToken,
        printLabelToken,
        equalsLabelToken,
        copyLabelToken
};

Token LABEL_TYPE = createLegacyTokenType(LABEL_TYPE_INIT);

LabelToken* createLabelToken(const char* name) {
    LabelToken* token = (LabelToken*) malloc(sizeof(LabelToken));
    setTokenType(&token->token_, LABEL_TYPE);
    SrcLoc loc;
    loc.line = 0;
    loc.column = 0;
    setTokenLocation(&token->token_, loc);
    token->name = name;
    return token;
}

void destroyAbsJumpToken(Token* self) {
    free((void*) ((AbsJumpToken*) self)->label);
}

void printAbsJumpToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("absJump: %s\n", ((AbsJumpToken*) self)->label);
}

uint8_t equalsAbsJumpToken(Token* self, Token* other) {
    return strcmp(((AbsJumpToken*) self)->label, ((AbsJumpToken*) other)->label) == 0;
}

Token* copyAbsJumpToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    AbsJumpToken* jump = (AbsJumpToken*) self;
    const char* label = newStr(jump->label);
    return (Token*) createAbsJumpToken(label);
}

LegacyTokenInit ABS_JUMP_TYPE_INIT = {
        TOKEN_ABS_JUMP,
        destroyAbsJumpToken,
        printAbsJumpToken,
        equalsAbsJumpToken,
        copyAbsJumpToken
};

Token ABS_JUMP_TYPE = createLegacyTokenType(ABS_JUMP_TYPE_INIT);

AbsJumpToken* createAbsJumpToken(const char* label) {
    AbsJumpToken* token = (AbsJumpToken*) malloc(sizeof(AbsJumpToken));
    setTokenType(&token->token_, ABS_JUMP_TYPE);
    SrcLoc loc;
    loc.line = 0;
    loc.column = 0;
    setTokenLocation(&token->token_, loc);
    token->label = label;
    return token;
}

void destroyCondJumpToken(Token* self) {
    CondJumpToken* token = (CondJumpToken*) self;
    destroyToken(token->condition);
    free((void*) token->label);
}

void printCondJumpToken(Token* self, uint8_t indent) {
    CondJumpToken* token = (CondJumpToken*) self;
    printf("condJump: %s, %i\n", token->label, token->when);
    printTokenWithIndent(token->condition, indent + 1);
}

uint8_t equalsCondJumpToken(Token* self, Token* other) {
    CondJumpToken* a = (CondJumpToken*) self;
    CondJumpToken* b = (CondJumpToken*) other;
    return tokensEqual(a->condition, b->condition) && strcmp(a->label, b->label) == 0 &&
            a->when == b->when;
}

Token* copyCondJumpToken(Token* self, CopyVisitor visitor, void* data) {
    CondJumpToken* jump = (CondJumpToken*) self;
    Token* condition = visitor(jump->condition, data);
    const char* label = newStr(jump->label);
    return (Token*) createCondJumpToken(tokenLocation(self), condition, label,
            jump->when);
}

LegacyTokenInit COND_JUMP_TYPE_INIT = {
        TOKEN_COND_JUMP,
        destroyCondJumpToken,
        printCondJumpToken,
        equalsCondJumpToken,
        copyCondJumpToken
};

Token COND_JUMP_TYPE = createLegacyTokenType(COND_JUMP_TYPE_INIT);

CondJumpToken* createCondJumpToken(SrcLoc loc, Token* cond, const char* label,
        uint8_t when) {
    CondJumpToken* token = (CondJumpToken*) malloc(sizeof(CondJumpToken));
    setTokenType(&token->token_, COND_JUMP_TYPE);
    setTokenLocation(&token->token_, loc);
    token->condition = cond;
    token->label = label;
    token->when = when;
    return token;
}

void destroyPushBuiltinToken(Token* self) {
    PushBuiltinToken* builtin = (PushBuiltinToken*) self;
    free((char*) builtin->name);
}

void printPushBuiltinToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    PushBuiltinToken* builtin = (PushBuiltinToken*) self;
    printf("push_builtin: %s\n", builtin->name);
}

uint8_t equalsPushBuiltinToken(Token* self, Token* other) {
    PushBuiltinToken* builtin1 = (PushBuiltinToken*) self;
    PushBuiltinToken* builtin2 = (PushBuiltinToken*) other;
    return strcmp(builtin1->name, builtin2->name) == 0;
}

Token* copyPushBuiltinToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    PushBuiltinToken* builtin = (PushBuiltinToken*) self;
    return (Token*) createPushBuiltinToken(tokenLocation(self), newStr(builtin->name));
}

LegacyTokenInit PUSH_BUILTIN_TYPE_INIT = {
        TOKEN_PUSH_BUILTIN,
        destroyPushBuiltinToken,
        printPushBuiltinToken,
        equalsPushBuiltinToken,
        copyPushBuiltinToken
};

Token PUSH_BUILTIN_TYPE = createLegacyTokenType(PUSH_BUILTIN_TYPE_INIT);

PushBuiltinToken* createPushBuiltinToken(SrcLoc loc, const char* name) {
    PushBuiltinToken* builtin = (PushBuiltinToken*) malloc(sizeof(PushBuiltinToken));
    setTokenType(&builtin->token_, PUSH_BUILTIN_TYPE);
    setTokenLocation(&builtin->token_, loc);
    builtin->name = name;
    return builtin;
}

void destroyPushIntToken(Token* self) {
    UNUSED(self);
}

void printPushIntToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    PushIntToken* push = (PushIntToken*) self;
    printf("push_int: %i\n", push->value);
}

uint8_t equalsPushIntToken(Token* self, Token* other) {
    PushIntToken* push1 = (PushIntToken*) self;
    PushIntToken* push2 = (PushIntToken*) other;
    return push1->value == push2->value;
}

Token* copyPushIntToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    PushIntToken* push = (PushIntToken*) self;
    return (Token*) createPushIntToken(tokenLocation(self), push->value);
}

LegacyTokenInit PUSH_INT_TYPE_INIT = {
        TOKEN_PUSH_INT,
        destroyPushIntToken,
        printPushIntToken,
        equalsPushIntToken,
        copyPushIntToken
};

Token PUSH_INT_TYPE = createLegacyTokenType(PUSH_INT_TYPE_INIT);

PushIntToken* createPushIntToken(SrcLoc loc, int32_t value) {
    PushIntToken* push = (PushIntToken*) malloc(sizeof(PushIntToken));
    setTokenType(&push->token_, PUSH_INT_TYPE);
    setTokenLocation(&push->token_, loc);
    push->value = value;
    return push;
}

void destroyCallOpToken(Token* self) {
    UNUSED(self);
}

void printCallOpToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    CallOpToken* call = (CallOpToken*) self;
    printf("op_call: %i\n", call->arity);
}

uint8_t equalsCallOpToken(Token* self, Token* other) {
    CallOpToken* call1 = (CallOpToken*) self;
    CallOpToken* call2 = (CallOpToken*) other;
    return call1->arity == call2->arity;
}

Token* copyCallOpToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    CallOpToken* call = (CallOpToken*) self;
    return (Token*) createCallOpToken(tokenLocation(self), call->arity);
}

LegacyTokenInit CALL_OP_TYPE_INIT = {
        TOKEN_OP_CALL,
        destroyCallOpToken,
        printCallOpToken,
        equalsCallOpToken,
        copyCallOpToken
};

Token CALL_OP_TYPE = createLegacyTokenType(CALL_OP_TYPE_INIT);

CallOpToken* createCallOpToken(SrcLoc loc, uint8_t arity) {
    CallOpToken* call = (CallOpToken*) malloc(sizeof(CallOpToken));
    setTokenType(&call->token_, CALL_OP_TYPE);
    setTokenLocation(&call->token_, loc);
    call->arity = arity;
    return call;
}

void destroyStoreToken(Token* self) {
    StoreToken* store = (StoreToken*) self;
    free((char*) store->name);
}

void printStoreToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    StoreToken* store = (StoreToken*) self;
    printf("store: %s\n", store->name);
}

uint8_t equalsStoreToken(Token* self, Token* other) {
    StoreToken* store1 = (StoreToken*) self;
    StoreToken* store2 = (StoreToken*) other;
    return strcmp(store1->name, store2->name) == 0;
}

Token* copyStoreToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    StoreToken* store = (StoreToken*) self;
    return (Token*) createStoreToken(tokenLocation(self), newStr(store->name));
}

LegacyTokenInit STORE_TYPE_INIT = {
        TOKEN_STORE,
        destroyStoreToken,
        printStoreToken,
        equalsStoreToken,
        copyStoreToken
};

Token STORE_TYPE = createLegacyTokenType(STORE_TYPE_INIT);

StoreToken* createStoreToken(SrcLoc loc, const char* name) {
    StoreToken* store = (StoreToken*) malloc(sizeof(StoreToken));
    setTokenType(&store->token_, STORE_TYPE);
    setTokenLocation(&store->token_, loc);
    store->name = name;
    return store;
}

void destroyDupToken(Token* self) {
    UNUSED(self);
}

void printDupToken(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("dup\n");
}

uint8_t equalsDupToken(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* copyDupToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createDupToken(tokenLocation(self));
}

LegacyTokenInit DUP_TYPE_INIT = {
        TOKEN_DUP,
        destroyDupToken,
        printDupToken,
        equalsDupToken,
        copyDupToken
};

Token DUP_TYPE = createLegacyTokenType(DUP_TYPE_INIT);

DupToken* createDupToken(SrcLoc loc) {
    DupToken* dup = (DupToken*) malloc(sizeof(DupToken));
    setTokenType(&dup->token_, DUP_TYPE);
    setTokenLocation(&dup->token_, loc);
    return dup;
}

void destroyPushToken(Token* self) {
    PushToken* push = (PushToken*) self;
    destroyToken(push->value);
}

void printPushToken(Token* self, uint8_t ident) {
    PushToken* push = (PushToken*) self;
    printf("push:\n");
    printTokenWithIndent(push->value, ident + 1);
}

uint8_t equalsPushToken(Token* self, Token* other) {
    PushToken* push1 = (PushToken*) self;
    PushToken* push2 = (PushToken*) other;
    return tokensEqual(push1->value, push2->value);
}

Token* copyPushToken(Token* self, CopyVisitor visitor, void* data) {
    PushToken* push = (PushToken*) self;
    return (Token*) createPushToken(tokenLocation(self), visitor(push->value, data));
}

LegacyTokenInit PUSH_TYPE_INIT = {
        TOKEN_PUSH,
        destroyPushToken,
        printPushToken,
        equalsPushToken,
        copyPushToken
};

Token PUSH_TYPE = createLegacyTokenType(PUSH_TYPE_INIT);

PushToken* createPushToken(SrcLoc loc, Token* value) {
    PushToken* push = (PushToken*) malloc(sizeof(PushToken));
    setTokenType(&push->token_, PUSH_TYPE);
    setTokenLocation(&push->token_, loc);
    push->value = value;
    return push;
}

void destroyRot3Token(Token* self) {
    UNUSED(self);
}

void printRot3Token(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("rot3\n");
}

uint8_t equalsRot3Token(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* copyRot3Token(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createRot3Token(tokenLocation(self));
}

LegacyTokenInit ROT3_TYPE_INIT = {
        TOKEN_ROT3,
        destroyRot3Token,
        printRot3Token,
        equalsRot3Token,
        copyRot3Token
};

Token ROT3_TYPE = createLegacyTokenType(ROT3_TYPE_INIT);

Rot3Token* createRot3Token(SrcLoc location) {
    Rot3Token* rot = (Rot3Token*) malloc(sizeof(Rot3Token));
    setTokenType(&rot->token_, ROT3_TYPE);
    setTokenLocation(&rot->token_, location);
    return rot;
}

void destroyBuiltinToken(Token* self) {
    BuiltinToken* builtin = (BuiltinToken*) self;
    free((char*) builtin->name);
}

void printBuiltinToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    BuiltinToken* builtin = (BuiltinToken*) self;
    printf("builtin: %s\n", builtin->name);
}

uint8_t equalsBuiltinToken(Token* self, Token* other) {
    BuiltinToken* builtin1 = (BuiltinToken*) self;
    BuiltinToken* builtin2 = (BuiltinToken*) other;
    return strcmp(builtin1->name, builtin2->name) == 0;
}

Token* copyBuiltinToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    BuiltinToken* builtin = (BuiltinToken*) self;
    return (Token*) createBuiltinToken(tokenLocation(self), newStr(builtin->name));
}

LegacyTokenInit BUILTIN_TYPE_INIT = {
        TOKEN_BUILTIN,
        destroyBuiltinToken,
        printBuiltinToken,
        equalsBuiltinToken,
        copyBuiltinToken
};

Token BUILTIN_TYPE = createLegacyTokenType(BUILTIN_TYPE_INIT);

BuiltinToken* createBuiltinToken(SrcLoc loc, const char* name) {
    BuiltinToken* builtin = (BuiltinToken*) malloc(sizeof(BuiltinToken));
    setTokenType(&builtin->token_, BUILTIN_TYPE);
    setTokenLocation(&builtin->token_, loc);
    builtin->name = name;
    return builtin;
}

void destroySwapToken(Token* self) {
    UNUSED(self);
}

void printSwapToken(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("swap\n");
}

uint8_t equalsSwapToken(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* copySwapToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createSwapToken(tokenLocation(self));
}

LegacyTokenInit SWAP_TYPE_INIT = {
        TOKEN_SWAP,
        destroySwapToken,
        printSwapToken,
        equalsSwapToken,
        copySwapToken
};

Token SWAP_TYPE = createLegacyTokenType(SWAP_TYPE_INIT);

SwapToken* createSwapToken(SrcLoc loc) {
    SwapToken* swap = (SwapToken*) malloc(sizeof(SwapToken));
    setTokenType(&swap->token_, SWAP_TYPE);
    setTokenLocation(&swap->token_, loc);
    return swap;
}

void destroyPopToken(Token* self) {
    UNUSED(self);
}

void printPopToken(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("pop\n");
}

uint8_t equalsPopToken(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* copyPopToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createPopToken(tokenLocation(self));
}

LegacyTokenInit POP_TYPE_INIT = {
        TOKEN_POP,
        destroyPopToken,
        printPopToken,
        equalsPopToken,
        copyPopToken
};

Token POP_TYPE = createLegacyTokenType(POP_TYPE_INIT);

PopToken* createPopToken(SrcLoc loc) {
    PopToken* pop = (PopToken*) malloc(sizeof(PopToken));
    setTokenType(&pop->token_, POP_TYPE);
    setTokenLocation(&pop->token_, loc);
    return pop;
}

void destroyCheckNoneToken(Token* self) {
    UNUSED(self);
}

void printCheckNoneToken(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("check_none\n");
}

uint8_t equalsCheckNoneToken(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* copyCheckNoneToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createCheckNoneToken(tokenLocation(self));
}

LegacyTokenInit CHECK_NONE_TYPE_INIT = {
        TOKEN_CHECK_NONE,
        destroyCheckNoneToken,
        printCheckNoneToken,
        equalsCheckNoneToken,
        copyCheckNoneToken
};

Token CHECK_NONE_TYPE = createLegacyTokenType(CHECK_NONE_TYPE_INIT);

CheckNoneToken* createCheckNoneToken(SrcLoc location) {
    CheckNoneToken* check = (CheckNoneToken*) malloc(sizeof(CheckNoneToken));
    setTokenType(&check->token_, CHECK_NONE_TYPE);
    setTokenLocation(&check->token_, location);
    return check;
}

void destroyNewFuncToken(Token* self) {
    NewFuncToken* func = (NewFuncToken*) self;
    free((char*) func->name);
}

void printNewFuncToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    NewFuncToken* func = (NewFuncToken*) self;
    printf("new_func: %s\n", func->name);
}

uint8_t equalsNewFuncToken(Token* self, Token* other) {
    NewFuncToken* func1 = (NewFuncToken*) self;
    NewFuncToken* func2 = (NewFuncToken*) other;
    return strcmp(func1->name, func2->name) == 0;
}

Token* copyNewFuncToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    NewFuncToken* func = (NewFuncToken*) self;
    return (Token*) createNewFuncToken(tokenLocation(self), newStr(func->name));
}

LegacyTokenInit NEW_FUNC_TYPE_INIT = {
        TOKEN_NEW_FUNC,
        destroyNewFuncToken,
        printNewFuncToken,
        equalsNewFuncToken,
        copyNewFuncToken
};

Token NEW_FUNC_TYPE = createLegacyTokenType(NEW_FUNC_TYPE_INIT);

NewFuncToken* createNewFuncToken(SrcLoc location, const char* name) {
    NewFuncToken* func = (NewFuncToken*) malloc(sizeof(NewFuncToken));
    setTokenType(&func->token_, NEW_FUNC_TYPE);
    setTokenLocation(&func->token_, location);
    func->name = name;
    return func;
}

/**
 * Frees a token's memory, it's data's memory and subtokens recursively
 */
void destroyToken(Token* token) {
    if(token == NULL) {
        return;
    }
    //token->destroy_(token);
    //free(token);
}

void destroyTokenVoid(void* token) {
    destroyToken((Token*) token);
}

void destroyIfBranch(void* x) {
    IfBranch* branch = (IfBranch*) x;
    destroyToken((Token*) branch->condition);
    destroyToken((Token*) branch->block);
    free(branch);
}

/**
 * Prints the given token. Subchildren are indented.
 */
void printTokenWithIndent(Token* token, uint8_t indent) {
    printIndent(indent);

    if(token == NULL) {
        printf("NULL\n");
    //} else if(token->print == NULL) {
    //    printf("unknown\n");
    } else {
        //token->print(token, indent);
        token->methods->print(token, indent);
    }
}

void printToken(Token* token) {
    printTokenWithIndent(token, 0);
}

/**
 * Determines if two tokens are equal.
 */
uint8_t tokensEqual(Token* a, Token* b) {
    //if(a == NULL || b == NULL || getTokenType(a) != getTokenType(b) ||
    //        a->equals == NULL || b->equals == NULL) {
    if(a == NULL || b == NULL || getTokenType(a) != getTokenType(b)) {
        return 0;
    }
    //return a->equals(a, b);
    return a->methods->equals(a, b);
}

uint8_t tokensEqualVoid(void* a, void* b) {
    return tokensEqual((Token*) a, (Token*) b);
}

uint8_t branchesEqual(void* a, void* b) {
    IfBranch* branchA = (IfBranch*) a;
    IfBranch* branchB = (IfBranch*) b;
    return tokensEqual((Token*) branchA->condition, (Token*) branchB->condition) &&
            tokensEqual((Token*) branchA->block, (Token*) branchB->block);
}

Token* copyToken(Token* token, CopyVisitor visitor, void* data) {
    //return token->copy(token, visitor, data);
    return token->methods->copy(token, visitor, data);
}
