#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "main/tokens.h"
#include "main/util.h"

#define UNUSED(x) (void)(x)
#define tokenInstanceFields {0, 0}

uint8_t tokensEqualVoid(void* a, void* b);
uint8_t branchesEqual(void* a, void* b);

SrcLoc tokenLocation(Token* token) {
    return token->location;
}

TokenType getTokenType(Token* token) {
    return token->type();
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

IntToken::IntToken(SrcLoc location, int32_t value) :
    Token(location), value(value) {}

TokenType IntToken::type() {
    return TOKEN_INT;
}

void IntToken::destroy(Token* self) {
    UNUSED(self);
}

void IntToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("int: %i\n", getIntTokenValue((IntToken*) self));
}

uint8_t IntToken::equals(Token* self, Token* other) {
    return getIntTokenValue((IntToken*) self) == getIntTokenValue((IntToken*) other);
}

Token* IntToken::copy(Token* token, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    IntToken* intToken = (IntToken*) token;
    return (Token*) createIntToken(tokenLocation((Token*) intToken),
            getIntTokenValue(intToken));
}

int32_t getIntTokenValue(IntToken* token) {
    return token->value;
}

/**
 * constructs a integer token
 *
 * @param value the token's value
 * @return the newly created token
 */
IntToken* createIntToken(SrcLoc loc, int32_t value) {
    return new IntToken(loc, value);
}

FloatToken::FloatToken(SrcLoc location, float value) :
    Token(location), value(value) {}

TokenType FloatToken::type() {
    return TOKEN_FLOAT;
}

void FloatToken::destroy(Token* self) {
    UNUSED(self);
}

void FloatToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("float: %f\n", getFloatTokenValue((FloatToken*) self));
}

uint8_t FloatToken::equals(Token* self, Token* other) {
    return getFloatTokenValue(((FloatToken*) self)) == getFloatTokenValue(((FloatToken*) other));
}

Token* FloatToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    FloatToken* num = (FloatToken*) self;
    return (Token*) createFloatToken(tokenLocation(self), getFloatTokenValue(num));
}

float getFloatTokenValue(FloatToken* token) {
    return token->value;
}

FloatToken* createFloatToken(SrcLoc location, float value) {
    return new FloatToken(location, value);
}

LiteralToken::LiteralToken(SrcLoc location, const char* value) :
    Token(location), value(value) {}

TokenType LiteralToken::type() {
    return TOKEN_LITERAL;
}

void LiteralToken::destroy(Token* self) {
    //free((void*) ((LiteralToken*) self)->value);
}

void LiteralToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("literal: %s\n", getLiteralTokenValue((LiteralToken*) self));
}

uint8_t LiteralToken::equals(Token* self, Token* other) {
    return strcmp(getLiteralTokenValue((LiteralToken*) self),
            getLiteralTokenValue((LiteralToken*) other)) == 0;
}

Token* LiteralToken::copy(Token* token, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    LiteralToken* literal = (LiteralToken*) token;
    const char* copied = newStr(getLiteralTokenValue(literal));
    return (Token*) createLiteralToken(tokenLocation((Token*) literal), copied);
}

const char* getLiteralTokenValue(LiteralToken* token) {
    return token->value;
}

/**
 * constructs a literal token
 *
 * @param value a unique reference to the token's value.
 * @return the newly created token
 */
LiteralToken* createLiteralToken(SrcLoc loc, const char* value) {
    return new LiteralToken(loc, value);
}

IdentifierToken::IdentifierToken(SrcLoc location, const char* value) :
    Token(location), value(value) {}

TokenType IdentifierToken::type() {
    return TOKEN_IDENTIFIER;
}

void IdentifierToken::destroy(Token* token) {
    //free((void*) ((IdentifierToken*) token)->value);
}

void IdentifierToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("identifier: %s\n", (getIdentifierTokenValue((IdentifierToken*) self)));
}

uint8_t IdentifierToken::equals(Token* self, Token* other) {
    return strcmp(getIdentifierTokenValue((IdentifierToken*) self),
            getIdentifierTokenValue((IdentifierToken*) other)) == 0;
}

Token* IdentifierToken::copy(Token* token, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    IdentifierToken* identifier = (IdentifierToken*) token;
    SrcLoc loc = tokenLocation((Token*) identifier);
    return (Token*) createIdentifierToken(loc, newStr(getIdentifierTokenValue(identifier)));
}

const char* getIdentifierTokenValue(IdentifierToken* token) {
    return token->value;
}

/**
 * constructs an identifier token
 *
 * @param value a unique reference to the token's name.
 * @return the newly created token
 */
IdentifierToken* createIdentifierToken(SrcLoc loc, const char* value) {
    return new IdentifierToken(loc, value);
}

TupleToken::TupleToken(SrcLoc location, List* elements) :
    Token(location), elements(elements) {}

TokenType TupleToken::type() {
    return TOKEN_TUPLE;
}

void TupleToken::destroy(Token* self) {
    //TupleToken* tuple = (TupleToken*) self;
    //destroyList(tuple->elements, destroyTokenVoid);
}

void TupleToken::print(Token* self, uint8_t indent) {
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

uint8_t TupleToken::equals(Token* self, Token* other) {
    TupleToken* tuple1 = (TupleToken*) self;
    TupleToken* tuple2 = (TupleToken*) other;
    return allList2(getTupleTokenElements(tuple1), getTupleTokenElements(tuple2), tokensEqualVoid);
}

Token* TupleToken::copy(Token* self, CopyVisitor visitor, void* data) {
    TupleToken* tuple = (TupleToken*) self;
    List* copied = copyTokenList(getTupleTokenElements(tuple), visitor, data);
    return (Token*) createTupleToken(tokenLocation((Token*) tuple), copied);
}

List* getTupleTokenElements(TupleToken* token) {
    return token->elements;
}

TupleToken* createTupleToken(SrcLoc location, List* elements) {
    return new TupleToken(location, elements);
}

ListToken::ListToken(SrcLoc location, List* elements) :
    Token(location), elements(elements) {}

TokenType ListToken::type() {
    return TOKEN_LIST;
}

void ListToken::destroy(Token* self) {
    //ListToken* list = (ListToken*) self;
    //destroyList(list->elements, destroyTokenVoid);
}

void ListToken::print(Token* self, uint8_t indent) {
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

uint8_t ListToken::equals(Token* self, Token* other) {
    ListToken* list1 = (ListToken*) self;
    ListToken* list2 = (ListToken*) other;
    return allList2(getListTokenElements(list1), getListTokenElements(list2),
            tokensEqualVoid);
}

Token* ListToken::copy(Token* self, CopyVisitor visitor, void* data) {
    ListToken* list = (ListToken*) self;
    List* copied = copyTokenList(getListTokenElements(list), visitor, data);
    return (Token*) createListToken(tokenLocation((Token*) list), copied);
}

List* getListTokenElements(ListToken* token) {
    return token->elements;
}

ListToken* createListToken(SrcLoc location, List* elements) {
    return new ListToken(location, elements);
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

ObjectToken::ObjectToken(SrcLoc location, List* elements) :
    Token(location), elements(elements) {}

TokenType ObjectToken::type() {
    return TOKEN_OBJECT;
}

void ObjectToken::destroy(Token* self) {
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

void ObjectToken::print(Token* self, uint8_t indent) {
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

uint8_t ObjectToken::equals(Token* self, Token* other) {
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

Token* ObjectToken::copy(Token* self, CopyVisitor visitor, void* data) {
    ObjectToken* object = (ObjectToken*) self;
    List* copied = copyObjectPairs(getObjectTokenElements(object), visitor, data);
    return (Token*) createObjectToken(tokenLocation(self), copied);
}

List* getObjectTokenElements(ObjectToken* token) {
    return token->elements;
}

ObjectToken* createObjectToken(SrcLoc location, List* elements) {
    return new ObjectToken(location, elements);
}

CallToken::CallToken(SrcLoc location, List* children) :
    Token(location), children(children) {}

TokenType CallToken::type() {
    return TOKEN_CALL;
}

void CallToken::destroy(Token* self) {
    List* children = getCallTokenChildren((CallToken*) self);
    while(children != NULL) {
        destroyToken((Token*) children->head);
        children = children->tail;
    }
    destroyShallowList(getCallTokenChildren((CallToken*) self));
}

void CallToken::print(Token* self, uint8_t indent) {
    printf("call:\n");
    List* children = getCallTokenChildren((CallToken*) self);
    while(children != NULL) {
        printTokenWithIndent((Token*) children->head, indent + 1);
        children = children->tail;
    }
}

uint8_t CallToken::equals(Token* self, Token* other) {
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

Token* CallToken::copy(Token* self, CopyVisitor visitor, void* data) {
    CallToken* call = (CallToken*) self;
    List* children = copyTokenList(getCallTokenChildren(call), visitor, data);
    return (Token*) createCallToken(tokenLocation((Token*) call), children);
}

List* getCallTokenChildren(CallToken* token) {
    return token->children;
}

CallToken* createCallToken(SrcLoc location, List* children) {
    return new CallToken(location, children);
}

BinaryOpToken::BinaryOpToken(SrcLoc location, const char* op, Token* left, Token* right) :
    Token(location), op(op), left(left), right(right) {}

TokenType BinaryOpToken::type() {
    return TOKEN_BINARY_OP;
}

void BinaryOpToken::destroy(Token* self) {
    BinaryOpToken* binaryOp = (BinaryOpToken*) self;
    free((void*) getBinaryOpTokenOp(binaryOp));
    destroyToken(getBinaryOpTokenLeft(binaryOp));
    destroyToken(getBinaryOpTokenRight(binaryOp));
}

void BinaryOpToken::print(Token* self, uint8_t indent) {
    BinaryOpToken* binaryOp = (BinaryOpToken*) self;
    printf("binaryOp: '%s'\n", getBinaryOpTokenOp(binaryOp));
    printTokenWithIndent(getBinaryOpTokenLeft(binaryOp), indent + 1);
    printTokenWithIndent(getBinaryOpTokenRight(binaryOp), indent + 1);
}

uint8_t BinaryOpToken::equals(Token* self, Token* other) {
    BinaryOpToken* selfBinOp = (BinaryOpToken*) self;
    BinaryOpToken* otherBinOp = (BinaryOpToken*) other;
    return strcmp(getBinaryOpTokenOp(selfBinOp), getBinaryOpTokenOp(otherBinOp)) == 0 &&
            tokensEqual(getBinaryOpTokenLeft(selfBinOp), getBinaryOpTokenLeft(otherBinOp)) &&
            tokensEqual(getBinaryOpTokenRight(selfBinOp), getBinaryOpTokenRight(otherBinOp));
}

Token* BinaryOpToken::copy(Token* self, CopyVisitor visitor, void* data) {
    BinaryOpToken* binOp = (BinaryOpToken*) self;
    const char* op = newStr(getBinaryOpTokenOp(binOp));
    Token* left = visitor(getBinaryOpTokenLeft(binOp), data);
    Token* right = visitor(getBinaryOpTokenRight(binOp), data);
    return (Token*) createBinaryOpToken(tokenLocation((Token*) binOp), op, left, right);
}

const char* getBinaryOpTokenOp(BinaryOpToken* token) {
    return token->op;
}

Token* getBinaryOpTokenLeft(BinaryOpToken* token) {
    return token->left;
}

Token* getBinaryOpTokenRight(BinaryOpToken* token) {
    return token->right;
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
    return new BinaryOpToken(loc, op, left, right);
}

UnaryOpToken::UnaryOpToken(SrcLoc location, const char* op, Token* child) :
    Token(location), op(op), child(child) {}

TokenType UnaryOpToken::type() {
    return TOKEN_UNARY_OP;
}

void UnaryOpToken::destroy(Token* self) {
    UnaryOpToken* unaryOp = (UnaryOpToken*) self;
    free((void*) getUnaryOpTokenOp(unaryOp));
    destroyToken(getUnaryOpTokenChild(unaryOp));
}

void UnaryOpToken::print(Token* self, uint8_t indent) {
    UnaryOpToken* unaryOp = (UnaryOpToken*) self;
    printf("unaryOp: %s\n", getUnaryOpTokenOp(unaryOp));
    printTokenWithIndent(getUnaryOpTokenChild(unaryOp), indent + 1);
}

uint8_t UnaryOpToken::equals(Token* self, Token* other) {
    UnaryOpToken* selfUnOp = (UnaryOpToken*) self;
    UnaryOpToken* otherUnOp = (UnaryOpToken*) other;
    return strcmp(getUnaryOpTokenOp(selfUnOp), getUnaryOpTokenOp(otherUnOp)) == 0 &&
            tokensEqual(getUnaryOpTokenChild(selfUnOp), getUnaryOpTokenChild(otherUnOp));
}

Token* UnaryOpToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UnaryOpToken* unOp = (UnaryOpToken*) self;
    return (Token*) createUnaryOpToken(tokenLocation(self),
            newStr(getUnaryOpTokenOp(unOp)),
            visitor(getUnaryOpTokenChild(unOp), data));
}

const char* getUnaryOpTokenOp(UnaryOpToken* token) {
    return token->op;
}

Token* getUnaryOpTokenChild(UnaryOpToken* token) {
    return token->child;
}

/**
 * constructs a unary op token
 *
 * @param op a unique reference to the token's operation.
 * @param child a unique reference to the token's operand.
 * @return the newly created token
 */
UnaryOpToken* createUnaryOpToken(SrcLoc loc, const char* op, Token* child) {
    return new UnaryOpToken(loc, op, child);
}

AssignmentToken::AssignmentToken(SrcLoc location, Token* left, Token* right) :
    Token(location), left(left), right(right) {}

TokenType AssignmentToken::type() {
    return TOKEN_ASSIGNMENT;
}

void AssignmentToken::destroy(Token* self) {
    //AssignmentToken* assignment = (AssignmentToken*) self;
    //destroyToken((Token*) assignment->left);
    //destroyToken(assignment->right);
}

void AssignmentToken::print(Token* self, uint8_t indent) {
    AssignmentToken* assignment = (AssignmentToken*) self;
    printf("assignment:\n");

    printIndent(indent + 1);
    printf("lvalue:\n");
    printTokenWithIndent(getAssignmentTokenLeft(assignment), indent + 2);

    printIndent(indent + 1);
    printf("rvalue:\n");
    printTokenWithIndent(getAssignmentTokenRight(assignment), indent + 2);
}

uint8_t AssignmentToken::equals(Token* self, Token* other) {
    AssignmentToken* selfAssign = (AssignmentToken*) self;
    AssignmentToken* otherAssign = (AssignmentToken*) other;

    Token* selfLeft = getAssignmentTokenLeft(selfAssign);
    Token* selfRight = getAssignmentTokenRight(otherAssign);

    Token* otherLeft = getAssignmentTokenLeft(otherAssign);
    Token* otherRight = getAssignmentTokenRight(otherAssign);

    return tokensEqual(selfLeft, otherLeft) &&
            tokensEqual(selfRight, otherRight);
}

Token* AssignmentToken::copy(Token* self, CopyVisitor visitor, void* data) {
    AssignmentToken* assign = (AssignmentToken*) self;
    Token* left = visitor((Token*) getAssignmentTokenLeft(assign), data);
    Token* right = visitor(getAssignmentTokenRight(assign), data);
    return (Token*) createAssignmentToken(tokenLocation(self), left, right);
}

Token* getAssignmentTokenLeft(AssignmentToken* token) {
    return token->left;
}

Token* getAssignmentTokenRight(AssignmentToken* token) {
    return token->right;
}

/**
 * constructs an assignment token
 *
 * @param left a unique reference to the token's lvalue.
 * @param right a unique reference to the token's rvalue.
 */
AssignmentToken* createAssignmentToken(SrcLoc loc, Token* left, Token* right) {
    return new AssignmentToken(loc, left, right);
}

BlockToken::BlockToken(SrcLoc location, List* children) :
    Token(location), children(children) {}

TokenType BlockToken::type() {
    return TOKEN_BLOCK;
}

void BlockToken::destroy(Token* self) {
    //destroyList(((BlockToken*) self)->children, destroyTokenVoid);
}

void BlockToken::print(Token* self, uint8_t indent) {
    BlockToken* block = (BlockToken*) self;
    printf("block:\n");
    for(List* node = getBlockTokenChildren(block); node != NULL; node = node->tail) {
        printTokenWithIndent((Token*) node->head, indent + 1);
    }
}

uint8_t BlockToken::equals(Token* self, Token* other) {
    return allList2(getBlockTokenChildren((BlockToken*) self),
            getBlockTokenChildren((BlockToken*) other), tokensEqualVoid);
}

Token* BlockToken::copy(Token* self, CopyVisitor visitor, void* data) {
    BlockToken* block = (BlockToken*) self;
    List* copied = copyTokenList(getBlockTokenChildren(block), visitor, data);
    SrcLoc location = tokenLocation(self);
    return (Token*) createBlockToken(location, copied);
}

List* getBlockTokenChildren(BlockToken* token) {
    return token->children;
}

BlockToken* createBlockToken(SrcLoc location, List* children) {
    return new BlockToken(location, children);
}

IfBranch* createIfBranch(Token* condition, BlockToken* block) {
    IfBranch* branch = (IfBranch*) malloc(sizeof(IfBranch));
    branch->condition = condition;
    branch->block = block;
    return branch;
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

IfToken::IfToken(SrcLoc location, List* branches, BlockToken* elseBranch) :
    Token(location), branches(branches), elseBranch(elseBranch) {}

TokenType IfToken::type() {
    return TOKEN_IF;
}

void IfToken::destroy(Token* self) {
    //IfToken* ifToken = (IfToken*) self;
    //destroyList(ifToken->branches, destroyIfBranch);
    //destroyToken((Token*) ifToken->elseBranch);
}

void IfToken::print(Token* self, uint8_t indent) {
    IfToken* ifStmt = (IfToken*) self;
    printf("if:\n");
    for(List* node = getIfTokenBranches(ifStmt); node != NULL; node = node->tail) {
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
    printTokenWithIndent((Token*) getIfTokenElseBranch(ifStmt), indent + 2);
}

uint8_t IfToken::equals(Token* self, Token* other) {
    IfToken* selfIf = (IfToken*) self;
    IfToken* otherIf = (IfToken*) other;

    if(!allList2(getIfTokenBranches(selfIf),
            getIfTokenBranches(otherIf), branchesEqual)) {
        return 0;
    }
    if(getIfTokenElseBranch(selfIf) == NULL &&
            getIfTokenElseBranch(otherIf) == NULL) {
        return 1;
    }
    return tokensEqual((Token*) getIfTokenElseBranch(selfIf),
            (Token*) getIfTokenElseBranch(otherIf));
}

Token* IfToken::copy(Token* self, CopyVisitor visitor, void* data) {
    IfToken* ifToken = (IfToken*) self;
    List* branches = copyIfBranches(getIfTokenBranches(ifToken), visitor, data);
    Token* elseBranch;
    if(getIfTokenElseBranch(ifToken) == NULL) {
        elseBranch = NULL;
    } else {
        elseBranch = visitor((Token*) getIfTokenElseBranch(ifToken), data);
    }
    return (Token*) createIfToken(tokenLocation(self), branches,
            (BlockToken*) elseBranch);
}

List* getIfTokenBranches(IfToken* token) {
    return token->branches;
}

BlockToken* getIfTokenElseBranch(IfToken* token) {
    return token->elseBranch;
}

IfToken* createIfToken(SrcLoc loc, List* branches, BlockToken* elseBranch) {
    return new IfToken(loc, branches, elseBranch);
}

WhileToken::WhileToken(SrcLoc location, Token* condition, BlockToken* body) :
    Token(location), condition(condition), body(body) {}

TokenType WhileToken::type() {
    return TOKEN_WHILE;
}

void WhileToken::destroy(Token* self) {
    WhileToken* whileToken = (WhileToken*) self;
    destroyToken((Token*) getWhileTokenCondition(whileToken));
    destroyToken((Token*) getWhileTokenBody(whileToken));
}

void WhileToken::print(Token* self, uint8_t indent) {
    WhileToken* whileStmt = (WhileToken*) self;
    printf("while:\n");

    printIndent(indent + 1);
    printf("condition:\n");
    printTokenWithIndent((Token*) getWhileTokenCondition(whileStmt), indent + 2);

    printIndent(indent + 1);
    printf("body:\n");
    printTokenWithIndent((Token*) getWhileTokenBody(whileStmt), indent + 2);
}

uint8_t WhileToken::equals(Token* self, Token* other) {
    WhileToken* selfWhile = (WhileToken*) self;
    WhileToken* otherWhile = (WhileToken*) other;

    Token* selfCondition = getWhileTokenCondition(selfWhile);
    Token* selfBody = (Token*) getWhileTokenBody(selfWhile);

    Token* otherCondition = getWhileTokenCondition(otherWhile);
    Token* otherBody = (Token*) getWhileTokenBody(otherWhile);

    return tokensEqual(selfCondition, otherCondition) &&
            tokensEqual(selfBody, otherBody);
}

Token* WhileToken::copy(Token* self, CopyVisitor visitor, void* data) {
    WhileToken* whileToken = (WhileToken*) self;
    Token* condition = visitor(getWhileTokenCondition(whileToken), data);
    BlockToken* body = (BlockToken*) visitor((Token*) getWhileTokenBody(whileToken), data);
    return (Token*) createWhileToken(tokenLocation(self), condition, body);
}

Token* getWhileTokenCondition(WhileToken* token) {
    return token->condition;
}

BlockToken* getWhileTokenBody(WhileToken* token) {
    return token->body;
}

WhileToken* createWhileToken(SrcLoc loc, Token* condition, BlockToken* body) {
    return new WhileToken(loc, condition, body);
}

FuncToken::FuncToken(SrcLoc loc, IdentifierToken* name, List* args, BlockToken* body) :
    Token(loc), name(name), args(args), body(body) {}

TokenType FuncToken::type() {
    return TOKEN_FUNC;
}

void FuncToken::destroy(Token* self) {
    //FuncToken* funcToken = (FuncToken*) self;
    //destroyToken((Token*) funcToken->name);
    //destroyList(funcToken->args, destroyTokenVoid);
    //destroyToken((Token*) funcToken->body);
}

void FuncToken::print(Token* self, uint8_t indent) {
    FuncToken* func = (FuncToken*) self;
    printf("func: %s", getIdentifierTokenValue(getFuncTokenName(func)));
    List* node = getFuncTokenArgs(func);
    while(node != NULL) {
        printf(" %s", getIdentifierTokenValue((IdentifierToken*) node->head));
        node = node->tail;
    }
    printf("\n");
    printTokenWithIndent((Token*) getFuncTokenBody(func), indent + 1);
}

uint8_t FuncToken::equals(Token* self, Token* other) {
    FuncToken* selfFunc = (FuncToken*) self;
    FuncToken* otherFunc = (FuncToken*) other;

    Token* selfName = (Token*) getFuncTokenName(selfFunc);
    List* selfArgs = getFuncTokenArgs(selfFunc);
    Token* selfBody = (Token*) getFuncTokenBody(selfFunc);

    Token* otherName = (Token*) getFuncTokenName(otherFunc);
    List* otherArgs = getFuncTokenArgs(otherFunc);
    Token* otherBody = (Token*) getFuncTokenBody(otherFunc);

    return tokensEqual(selfName, otherName) &&
            allList2(selfArgs, otherArgs, tokensEqualVoid) &&
            tokensEqual(selfBody, otherBody);
}

Token* FuncToken::copy(Token* self, CopyVisitor visitor, void* data) {
    FuncToken* func = (FuncToken*) self;
    IdentifierToken* name = (IdentifierToken*)
            visitor((Token*) getFuncTokenName(func), data);
    List* args = copyTokenList(getFuncTokenArgs(func), visitor, data);
    BlockToken* body = (BlockToken*) visitor((Token*) getFuncTokenBody(func), data);

    return (Token*) createFuncToken(tokenLocation(self), name, args, body);
}

IdentifierToken* getFuncTokenName(FuncToken* token) {
    return token->name;
}

void setFuncTokenName(FuncToken* token, IdentifierToken* name) {
    token->name = name;
}

List* getFuncTokenArgs(FuncToken* token) {
    return token->args;
}

BlockToken* getFuncTokenBody(FuncToken* token) {
    return token->body;
}

FuncToken* createFuncToken(SrcLoc loc, IdentifierToken* name, List* args,
        BlockToken* body) {
    return new FuncToken(loc, name, args, body);
}

ReturnToken::ReturnToken(SrcLoc location, Token* body) :
    Token(location), body(body) {}

TokenType ReturnToken::type() {
    return TOKEN_RETURN;
}

void ReturnToken::destroy(Token* self) {
    //destroyToken(((ReturnToken*) self)->body);
}

void ReturnToken::print(Token* self, uint8_t indent) {
    printf("return:\n");
    printTokenWithIndent(getReturnTokenBody((ReturnToken*) self), indent + 1);
}

uint8_t ReturnToken::equals(Token* self, Token* other) {
    return tokensEqual(getReturnTokenBody((ReturnToken*) self),
            getReturnTokenBody((ReturnToken*) other));
}

Token* ReturnToken::copy(Token* self, CopyVisitor visitor, void* data) {
    ReturnToken* token = (ReturnToken*) self;
    Token* copied = visitor(getReturnTokenBody(token), data);
    return (Token*) createReturnToken(tokenLocation(self), copied);
}

Token* getReturnTokenBody(ReturnToken* token) {
    return token->body;
}

ReturnToken* createReturnToken(SrcLoc location, Token* body) {
    return new ReturnToken(location, body);
}

LabelToken::LabelToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

TokenType LabelToken::type() {
    return TOKEN_LABEL;
}

void LabelToken::destroy(Token* self) {
    //free((void*) ((LabelToken*) self)->name);
}

void LabelToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("label: %s \n", getLabelTokenName((LabelToken*) self));
}

uint8_t LabelToken::equals(Token* self, Token* other) {
    return strcmp(getLabelTokenName((LabelToken*) self),
            getLabelTokenName((LabelToken*) other)) == 0;
}

Token* LabelToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    LabelToken* label = (LabelToken*) self;
    const char* name = newStr(getLabelTokenName(label));
    return (Token*) createLabelToken(name);
}

const char* getLabelTokenName(LabelToken* token) {
    return token->name;
}

LabelToken* createLabelToken(const char* name) {
    SrcLoc loc;
    loc.line = 0;
    loc.column = 0;
    return new LabelToken(loc, name);
}

AbsJumpToken::AbsJumpToken(SrcLoc location, const char* label) :
    Token(location), label(label) {}

TokenType AbsJumpToken::type() {
    return TOKEN_ABS_JUMP;
}

void AbsJumpToken::destroy(Token* self) {
    //free((void*) ((AbsJumpToken*) self)->label);
}

void AbsJumpToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("absJump: %s\n", getAbsJumpTokenLabel((AbsJumpToken*) self));
}

uint8_t AbsJumpToken::equals(Token* self, Token* other) {
    return strcmp(getAbsJumpTokenLabel((AbsJumpToken*) self),
            getAbsJumpTokenLabel((AbsJumpToken*) other)) == 0;
}

Token* AbsJumpToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    AbsJumpToken* jump = (AbsJumpToken*) self;
    const char* label = newStr(getAbsJumpTokenLabel(jump));
    return (Token*) createAbsJumpToken(label);
}

const char* getAbsJumpTokenLabel(AbsJumpToken* token) {
    return token->label;
}

AbsJumpToken* createAbsJumpToken(const char* label) {
    SrcLoc loc;
    loc.line = 0;
    loc.column = 0;
    return new AbsJumpToken(loc, label);
}

CondJumpToken::CondJumpToken(SrcLoc loc, Token* condition, const char* label, uint8_t when) :
    Token(loc), condition(condition), label(label), when(when) {}

TokenType CondJumpToken::type() {
    return TOKEN_COND_JUMP;
}

void CondJumpToken::destroy(Token* self) {
    //CondJumpToken* token = (CondJumpToken*) self;
    //destroyToken(token->condition);
    //free((void*) token->label);
}

void CondJumpToken::print(Token* self, uint8_t indent) {
    CondJumpToken* token = (CondJumpToken*) self;
    printf("condJump: %s, %i\n", getCondJumpTokenLabel(token),
            getCondJumpTokenWhen(token));
    printTokenWithIndent(getCondJumpTokenCondition(token), indent + 1);
}

uint8_t CondJumpToken::equals(Token* self, Token* other) {
    CondJumpToken* a = (CondJumpToken*) self;
    CondJumpToken* b = (CondJumpToken*) other;

    return tokensEqual(getCondJumpTokenCondition(a), getCondJumpTokenCondition(b)) &&
            strcmp(getCondJumpTokenLabel(a), getCondJumpTokenLabel(b)) == 0 &&
            getCondJumpTokenWhen(a) == getCondJumpTokenWhen(b);
}

Token* CondJumpToken::copy(Token* self, CopyVisitor visitor, void* data) {
    CondJumpToken* jump = (CondJumpToken*) self;
    Token* condition = visitor(getCondJumpTokenCondition(jump), data);
    const char* label = newStr(getCondJumpTokenLabel(jump));
    return (Token*) createCondJumpToken(tokenLocation(self), condition, label,
            getCondJumpTokenWhen(jump));
}

Token* getCondJumpTokenCondition(CondJumpToken* token) {
    return token->condition;
}

const char* getCondJumpTokenLabel(CondJumpToken* token) {
    return token->label;
}

uint8_t getCondJumpTokenWhen(CondJumpToken* token) {
    return token->when;
}

CondJumpToken* createCondJumpToken(SrcLoc loc, Token* cond, const char* label,
        uint8_t when) {
    return new CondJumpToken(loc, cond, label, when);
}

PushBuiltinToken::PushBuiltinToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

TokenType PushBuiltinToken::type() {
    return TOKEN_PUSH_BUILTIN;
}

void PushBuiltinToken::destroy(Token* self) {
    //PushBuiltinToken* builtin = (PushBuiltinToken*) self;
    //free((char*) builtin->name);
}

void PushBuiltinToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    PushBuiltinToken* builtin = (PushBuiltinToken*) self;
    printf("push_builtin: %s\n", getPushBuiltinTokenName(builtin));
}

uint8_t PushBuiltinToken::equals(Token* self, Token* other) {
    PushBuiltinToken* builtin1 = (PushBuiltinToken*) self;
    PushBuiltinToken* builtin2 = (PushBuiltinToken*) other;
    return strcmp(getPushBuiltinTokenName(builtin1),
            getPushBuiltinTokenName(builtin2)) == 0;
}

Token* PushBuiltinToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    PushBuiltinToken* builtin = (PushBuiltinToken*) self;
    return (Token*) createPushBuiltinToken(tokenLocation(self),
            newStr(getPushBuiltinTokenName(builtin)));
}

const char* getPushBuiltinTokenName(PushBuiltinToken* token) {
    return token->name;
}

PushBuiltinToken* createPushBuiltinToken(SrcLoc loc, const char* name) {
    return new PushBuiltinToken(loc, name);
}

PushIntToken::PushIntToken(SrcLoc location, int32_t value) :
    Token(location), value(value) {}

TokenType PushIntToken::type() {
    return TOKEN_PUSH_INT;
}

void PushIntToken::destroy(Token* self) {
    UNUSED(self);
}

void PushIntToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    PushIntToken* push = (PushIntToken*) self;
    printf("push_int: %i\n", getPushIntTokenValue(push));
}

uint8_t PushIntToken::equals(Token* self, Token* other) {
    PushIntToken* push1 = (PushIntToken*) self;
    PushIntToken* push2 = (PushIntToken*) other;
    return getPushIntTokenValue(push1) == getPushIntTokenValue(push2);
}

Token* PushIntToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    PushIntToken* push = (PushIntToken*) self;
    return (Token*) createPushIntToken(tokenLocation(self), getPushIntTokenValue(push));
}

int32_t getPushIntTokenValue(PushIntToken* token) {
    return token->value;
}

PushIntToken* createPushIntToken(SrcLoc loc, int32_t value) {
    return new PushIntToken(loc, value);
}

CallOpToken::CallOpToken(SrcLoc location, uint8_t arity) :
    Token(location), arity(arity) {}

TokenType CallOpToken::type() {
    return TOKEN_OP_CALL;
}

void CallOpToken::destroy(Token* self) {
    UNUSED(self);
}

void CallOpToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    CallOpToken* call = (CallOpToken*) self;
    printf("op_call: %i\n", getCallOpTokenArity(call));
}

uint8_t CallOpToken::equals(Token* self, Token* other) {
    CallOpToken* call1 = (CallOpToken*) self;
    CallOpToken* call2 = (CallOpToken*) other;
    return getCallOpTokenArity(call1) == getCallOpTokenArity(call2);
}

Token* CallOpToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    CallOpToken* call = (CallOpToken*) self;
    return (Token*) createCallOpToken(tokenLocation(self), getCallOpTokenArity(call));
}

uint8_t getCallOpTokenArity(CallOpToken* token) {
    return token->arity;
}

CallOpToken* createCallOpToken(SrcLoc loc, uint8_t arity) {
    return new CallOpToken(loc, arity);
}

StoreToken::StoreToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

TokenType StoreToken::type() {
    return TOKEN_STORE;
}

void StoreToken::destroy(Token* self) {
    //StoreToken* store = (StoreToken*) self;
    //free((char*) store->name);
}

void StoreToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    StoreToken* store = (StoreToken*) self;
    printf("store: %s\n", getStoreTokenName(store));
}

uint8_t StoreToken::equals(Token* self, Token* other) {
    StoreToken* store1 = (StoreToken*) self;
    StoreToken* store2 = (StoreToken*) other;
    return strcmp(getStoreTokenName(store1), getStoreTokenName(store2)) == 0;
}

Token* StoreToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    StoreToken* store = (StoreToken*) self;
    return (Token*) createStoreToken(tokenLocation(self),
            newStr(getStoreTokenName(store)));
}

const char* getStoreTokenName(StoreToken* token) {
    return token->name;
}

StoreToken* createStoreToken(SrcLoc loc, const char* name) {
    return new StoreToken(loc, name);
}

DupToken::DupToken(SrcLoc location) :
    Token(location) {}

TokenType DupToken::type() {
    return TOKEN_DUP;
}

void DupToken::destroy(Token* self) {
    UNUSED(self);
}

void DupToken::print(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("dup\n");
}

uint8_t DupToken::equals(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* DupToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createDupToken(tokenLocation(self));
}

DupToken* createDupToken(SrcLoc loc) {
    return new DupToken(loc);
}

PushToken::PushToken(SrcLoc location, Token* value) :
    Token(location), value(value) {}

TokenType PushToken::type() {
    return TOKEN_PUSH;
}

void PushToken::destroy(Token* self) {
    //PushToken* push = (PushToken*) self;
    //destroyToken(push->value);
}

void PushToken::print(Token* self, uint8_t ident) {
    PushToken* push = (PushToken*) self;
    printf("push:\n");
    printTokenWithIndent(getPushTokenValue(push), ident + 1);
}

uint8_t PushToken::equals(Token* self, Token* other) {
    PushToken* push1 = (PushToken*) self;
    PushToken* push2 = (PushToken*) other;
    return tokensEqual(getPushTokenValue(push1), getPushTokenValue(push2));
}

Token* PushToken::copy(Token* self, CopyVisitor visitor, void* data) {
    PushToken* push = (PushToken*) self;
    return (Token*) createPushToken(tokenLocation(self),
            visitor(getPushTokenValue(push), data));
}

Token* getPushTokenValue(PushToken* token) {
    return token->value;
}

PushToken* createPushToken(SrcLoc loc, Token* value) {
    return new PushToken(loc, value);
}

Rot3Token::Rot3Token(SrcLoc location) :
    Token(location) {}

TokenType Rot3Token::type() {
    return TOKEN_ROT3;
}

void Rot3Token::destroy(Token* self) {
    UNUSED(self);
}

void Rot3Token::print(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("rot3\n");
}

uint8_t Rot3Token::equals(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* Rot3Token::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createRot3Token(tokenLocation(self));
}

Rot3Token* createRot3Token(SrcLoc location) {
    return new Rot3Token(location);
}

SwapToken::SwapToken(SrcLoc location) :
    Token(location) {}

TokenType SwapToken::type() {
    return TOKEN_SWAP;
}

void SwapToken::destroy(Token* self) {
    UNUSED(self);
}

void SwapToken::print(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("swap\n");
}

uint8_t SwapToken::equals(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* SwapToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createSwapToken(tokenLocation(self));
}

SwapToken* createSwapToken(SrcLoc loc) {
    return new SwapToken(loc);
}

PopToken::PopToken(SrcLoc location) :
    Token(location) {}

TokenType PopToken::type() {
    return TOKEN_POP;
}

void PopToken::destroy(Token* self) {
    UNUSED(self);
}

void PopToken::print(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("pop\n");
}

uint8_t PopToken::equals(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* PopToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createPopToken(tokenLocation(self));
}

PopToken* createPopToken(SrcLoc loc) {
    return new PopToken(loc);
}

BuiltinToken::BuiltinToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

TokenType BuiltinToken::type() {
    return TOKEN_BUILTIN;
}

void BuiltinToken::destroy(Token* self) {
    //BuiltinToken* builtin = (BuiltinToken*) self;
    //free((char*) builtin->name);
}

void BuiltinToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    BuiltinToken* builtin = (BuiltinToken*) self;
    printf("builtin: %s\n", getBuiltinTokenName(builtin));
}

uint8_t BuiltinToken::equals(Token* self, Token* other) {
    BuiltinToken* builtin1 = (BuiltinToken*) self;
    BuiltinToken* builtin2 = (BuiltinToken*) other;
    return strcmp(getBuiltinTokenName(builtin1), getBuiltinTokenName(builtin2)) == 0;
}

Token* BuiltinToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    BuiltinToken* builtin = (BuiltinToken*) self;
    return (Token*) createBuiltinToken(tokenLocation(self),
            newStr(getBuiltinTokenName(builtin)));
}

const char* getBuiltinTokenName(BuiltinToken* token) {
    return token->name;
}

BuiltinToken* createBuiltinToken(SrcLoc loc, const char* name) {
    return new BuiltinToken(loc, name);
}

CheckNoneToken::CheckNoneToken(SrcLoc location) :
    Token(location) {}

TokenType CheckNoneToken::type() {
    return TOKEN_CHECK_NONE;
}

void CheckNoneToken::destroy(Token* self) {
    UNUSED(self);
}

void CheckNoneToken::print(Token* self, uint8_t indent) {
    UNUSED(self);
    UNUSED(indent);
    printf("check_none\n");
}

uint8_t CheckNoneToken::equals(Token* self, Token* other) {
    UNUSED(self);
    UNUSED(other);
    return 1;
}

Token* CheckNoneToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createCheckNoneToken(tokenLocation(self));
}

CheckNoneToken* createCheckNoneToken(SrcLoc location) {
    return new CheckNoneToken(location);
}

NewFuncToken::NewFuncToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

TokenType NewFuncToken::type() {
    return TOKEN_NEW_FUNC;
}

void NewFuncToken::destroy(Token* self) {
    //NewFuncToken* func = (NewFuncToken*) self;
    //free((char*) func->name);
}

void NewFuncToken::print(Token* self, uint8_t indent) {
    UNUSED(indent);
    NewFuncToken* func = (NewFuncToken*) self;
    printf("new_func: %s\n", getNewFuncTokenName(func));
}

uint8_t NewFuncToken::equals(Token* self, Token* other) {
    NewFuncToken* func1 = (NewFuncToken*) self;
    NewFuncToken* func2 = (NewFuncToken*) other;
    return strcmp(getNewFuncTokenName(func1), getNewFuncTokenName(func2)) == 0;
}

Token* NewFuncToken::copy(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    NewFuncToken* func = (NewFuncToken*) self;
    return (Token*) createNewFuncToken(tokenLocation(self),
            newStr(getNewFuncTokenName(func)));
}

const char* getNewFuncTokenName(NewFuncToken* token) {
    return token->name;
}

NewFuncToken* createNewFuncToken(SrcLoc location, const char* name) {
    return new NewFuncToken(location, name);
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
    } else {
        token->print(token, indent);
    }
}

void printToken(Token* token) {
    printTokenWithIndent(token, 0);
}

/**
 * Determines if two tokens are equal.
 */
uint8_t tokensEqual(Token* a, Token* b) {
    if(a == NULL || b == NULL || getTokenType(a) != getTokenType(b)) {
        return 0;
    }
    return a->equals(a, b);
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
    return token->copy(token, visitor, data);
}
