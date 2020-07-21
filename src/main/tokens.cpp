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

void IntToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("int: %i\n", this->value);
}

uint8_t IntToken::equals(Token* other) {
    return this->value == ((IntToken*) other)->value;
}

Token* IntToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createIntToken(this->location, this->value);
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

void FloatToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("float: %f\n", this->value);
}

uint8_t FloatToken::equals(Token* other) {
    return this->value == ((FloatToken*) other)->value;
}

Token* FloatToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createFloatToken(this->location, this->value);
}

float getFloatTokenValue(FloatToken* token) {
    return token->value;
}

FloatToken* createFloatToken(SrcLoc location, float value) {
    return new FloatToken(location, value);
}

LiteralToken::LiteralToken(SrcLoc location, const char* value) :
    Token(location), value(value) {}

LiteralToken::~LiteralToken() {
    free((void*) this->value);
}

TokenType LiteralToken::type() {
    return TOKEN_LITERAL;
}

void LiteralToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("literal: %s\n", this->value);
}

uint8_t LiteralToken::equals(Token* other) {
    return strcmp(this->value, ((LiteralToken*) other)->value) == 0;
}

Token* LiteralToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createLiteralToken(this->location, newStr(this->value));
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

IdentifierToken::~IdentifierToken() {
    free((void*) this->value);
}

TokenType IdentifierToken::type() {
    return TOKEN_IDENTIFIER;
}

void IdentifierToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("identifier: %s\n", this->value);
}

uint8_t IdentifierToken::equals(Token* other) {
    return strcmp(this->value, ((IdentifierToken*) other)->value) == 0;
}

Token* IdentifierToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createIdentifierToken(this->location, newStr(this->value));
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

TupleToken::~TupleToken() {
    destroyList(this->elements, destroyTokenVoid);
}

TokenType TupleToken::type() {
    return TOKEN_TUPLE;
}

void TupleToken::print(uint8_t indent) {
    printf("tuple:\n");
    List* elements = this->elements;
    uint8_t i = 0;

    while(elements != NULL) {
        printIndent(indent + 1);
        printf("%i:\n", i);
        printTokenWithIndent((Token*) elements->head, indent + 2);
        elements = elements->tail;
        i++;
    }
}

uint8_t TupleToken::equals(Token* other) {
    return allList2(this->elements, ((TupleToken*) other)->elements, tokensEqualVoid);
}

Token* TupleToken::copy(CopyVisitor visitor, void* data) {
    List* copied = copyTokenList(this->elements, visitor, data);
    return (Token*) createTupleToken(this->location, copied);
}

List* getTupleTokenElements(TupleToken* token) {
    return token->elements;
}

TupleToken* createTupleToken(SrcLoc location, List* elements) {
    return new TupleToken(location, elements);
}

ListToken::ListToken(SrcLoc location, List* elements) :
    Token(location), elements(elements) {}

ListToken::~ListToken() {
    destroyList(this->elements, destroyTokenVoid);
}

TokenType ListToken::type() {
    return TOKEN_LIST;
}

void ListToken::print(uint8_t indent) {
    printf("list:\n");
    List* elements = this->elements;
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

uint8_t ListToken::equals(Token* other) {
    return allList2(this->elements, ((ListToken*) other)->elements,
            tokensEqualVoid);
}

Token* ListToken::copy(CopyVisitor visitor, void* data) {
    List* copied = copyTokenList(this->elements, visitor, data);
    return (Token*) createListToken(this->location, copied);
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

ObjectToken::~ObjectToken() {
    List* elements = this->elements;

    while(elements != NULL) {
        ObjectPair* pair = (ObjectPair*) elements->head;
        destroyToken(pair->key);
        destroyToken(pair->value);
        free(pair);
        elements = elements->tail;
    }

    destroyShallowList(this->elements);
}

TokenType ObjectToken::type() {
    return TOKEN_OBJECT;
}

void ObjectToken::print(uint8_t indent) {
    printf("object:\n");

    List* elements = this->elements;
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

uint8_t ObjectToken::equals(Token* other) {
    List* elements1 = this->elements;
    List* elements2 = ((ObjectToken*) other)->elements;

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

Token* ObjectToken::copy(CopyVisitor visitor, void* data) {
    List* copied = copyObjectPairs(this->elements, visitor, data);
    return (Token*) createObjectToken(this->location, copied);
}

List* getObjectTokenElements(ObjectToken* token) {
    return token->elements;
}

ObjectToken* createObjectToken(SrcLoc location, List* elements) {
    return new ObjectToken(location, elements);
}

CallToken::CallToken(SrcLoc location, List* children) :
    Token(location), children(children) {}

CallToken::~CallToken() {
    List* children = this->children;
    while(children != NULL) {
        destroyToken((Token*) children->head);
        children = children->tail;
    }
    destroyShallowList(this->children);
}

TokenType CallToken::type() {
    return TOKEN_CALL;
}

void CallToken::print(uint8_t indent) {
    printf("call:\n");
    List* children = this->children;
    while(children != NULL) {
        printTokenWithIndent((Token*) children->head, indent + 1);
        children = children->tail;
    }
}

uint8_t CallToken::equals(Token* other) {
    List* childrenA = this->children;
    List* childrenB = ((CallToken*) other)->children;

    while(childrenA != NULL && childrenB != NULL) {
        if(!tokensEqual((Token*) childrenA->head, (Token*) childrenB->head)) {
            return 0;
        }
        childrenA = childrenA->tail;
        childrenB = childrenB->tail;
    }

    return childrenA == NULL && childrenB == NULL;
}

Token* CallToken::copy(CopyVisitor visitor, void* data) {
    List* children = copyTokenList(this->children, visitor, data);
    return (Token*) createCallToken(this->location, children);
}

List* getCallTokenChildren(CallToken* token) {
    return token->children;
}

CallToken* createCallToken(SrcLoc location, List* children) {
    return new CallToken(location, children);
}

BinaryOpToken::BinaryOpToken(SrcLoc location, const char* op, Token* left, Token* right) :
    Token(location), op(op), left(left), right(right) {}

BinaryOpToken::~BinaryOpToken() {
    free((void*) this->op);
    destroyToken(this->left);
    destroyToken(this->right);
}

TokenType BinaryOpToken::type() {
    return TOKEN_BINARY_OP;
}

void BinaryOpToken::print(uint8_t indent) {
    printf("binaryOp: '%s'\n", this->op);
    printTokenWithIndent(this->left, indent + 1);
    printTokenWithIndent(this->right, indent + 1);
}

uint8_t BinaryOpToken::equals(Token* other) {
    BinaryOpToken* otherBinOp = (BinaryOpToken*) other;
    return strcmp(this->op, otherBinOp->op) == 0 &&
            tokensEqual(this->left, otherBinOp->left) &&
            tokensEqual(this->right, otherBinOp->right);
}

Token* BinaryOpToken::copy(CopyVisitor visitor, void* data) {
    const char* op = newStr(this->op);
    Token* left = visitor(this->left, data);
    Token* right = visitor(this->right, data);
    return (Token*) createBinaryOpToken(this->location, op, left, right);
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

UnaryOpToken::~UnaryOpToken() {
    free((void*) this->op);
    destroyToken(this->child);
}

TokenType UnaryOpToken::type() {
    return TOKEN_UNARY_OP;
}

void UnaryOpToken::print(uint8_t indent) {
    printf("unaryOp: %s\n", this->op);
    printTokenWithIndent(this->child, indent + 1);
}

uint8_t UnaryOpToken::equals(Token* other) {
    UnaryOpToken* otherUnOp = (UnaryOpToken*) other;
    return strcmp(this->op, otherUnOp->op) == 0 &&
            tokensEqual(this->child, otherUnOp->child);
}

Token* UnaryOpToken::copy( CopyVisitor visitor, void* data) {
    return (Token*) createUnaryOpToken(this->location,
            newStr(this->op),
            visitor(this->child, data));
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

AssignmentToken::~AssignmentToken() {
    destroyToken(this->left);
    destroyToken(this->right);
}

TokenType AssignmentToken::type() {
    return TOKEN_ASSIGNMENT;
}

void AssignmentToken::print(uint8_t indent) {
    printf("assignment:\n");

    printIndent(indent + 1);
    printf("lvalue:\n");
    printTokenWithIndent(this->left, indent + 2);

    printIndent(indent + 1);
    printf("rvalue:\n");
    printTokenWithIndent(this->right, indent + 2);
}

uint8_t AssignmentToken::equals(Token* other) {
    AssignmentToken* otherAssign = (AssignmentToken*) other;

    return tokensEqual(this->left, otherAssign->left) &&
            tokensEqual(this->right, otherAssign->right);
}

Token* AssignmentToken::copy(CopyVisitor visitor, void* data) {
    Token* left = visitor((Token*) this->left, data);
    Token* right = visitor(this->right, data);
    return (Token*) createAssignmentToken(tokenLocation(this), left, right);
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

BlockToken::~BlockToken() {
    destroyList(this->children, destroyTokenVoid);
}

TokenType BlockToken::type() {
    return TOKEN_BLOCK;
}

void BlockToken::print(uint8_t indent) {
    printf("block:\n");
    for(List* node = this->children; node != NULL; node = node->tail) {
        printTokenWithIndent((Token*) node->head, indent + 1);
    }
}

uint8_t BlockToken::equals(Token* other) {
    return allList2(this->children, ((BlockToken*) other)->children,
            tokensEqualVoid);
}

Token* BlockToken::copy(CopyVisitor visitor, void* data) {
    List* copied = copyTokenList(this->children, visitor, data);
    return (Token*) createBlockToken(this->location, copied);
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

IfToken::~IfToken() {
    destroyList(this->branches, destroyIfBranch);
    destroyToken(this->elseBranch);
}

TokenType IfToken::type() {
    return TOKEN_IF;
}

void IfToken::print(uint8_t indent) {
    printf("if:\n");
    for(List* node = this->branches; node != NULL; node = node->tail) {
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
    printTokenWithIndent((Token*) this->elseBranch, indent + 2);
}

uint8_t IfToken::equals(Token* other) {
    IfToken* otherIf = (IfToken*) other;

    if(!allList2(this->branches, otherIf->branches, branchesEqual)) {
        return 0;
    }
    if(this->elseBranch == NULL && otherIf->elseBranch == NULL) {
        return 1;
    }
    return tokensEqual(this->elseBranch, otherIf->elseBranch);
}

Token* IfToken::copy(CopyVisitor visitor, void* data) {
    List* branches = copyIfBranches(this->branches, visitor, data);
    Token* elseBranch;
    if(this->elseBranch == NULL) {
        elseBranch = NULL;
    } else {
        elseBranch = visitor((Token*) this->elseBranch, data);
    }
    return (Token*) createIfToken(this->location, branches,
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

WhileToken::~WhileToken() {
    destroyToken(this->condition);
    destroyToken(this->body);
}

TokenType WhileToken::type() {
    return TOKEN_WHILE;
}

void WhileToken::print(uint8_t indent) {
    printf("while:\n");

    printIndent(indent + 1);
    printf("condition:\n");
    printTokenWithIndent(this->condition, indent + 2);

    printIndent(indent + 1);
    printf("body:\n");
    printTokenWithIndent(this->body, indent + 2);
}

uint8_t WhileToken::equals(Token* other) {
    WhileToken* otherWhile = (WhileToken*) other;

    return tokensEqual(this->condition, otherWhile->condition) &&
            tokensEqual(this->body, otherWhile->body);
}

Token* WhileToken::copy(CopyVisitor visitor, void* data) {
    Token* condition = visitor(this->condition, data);
    BlockToken* body = (BlockToken*) visitor((Token*) this->body, data);
    return (Token*) createWhileToken(this->location, condition, body);
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

FuncToken::~FuncToken() {
    destroyToken(this->name);
    destroyList(this->args, destroyTokenVoid);
    destroyToken(this->body);
}

TokenType FuncToken::type() {
    return TOKEN_FUNC;
}

void FuncToken::print(uint8_t indent) {
    printf("func: %s", getIdentifierTokenValue(this->name));
    List* node = this->args;
    while(node != NULL) {
        printf(" %s", getIdentifierTokenValue((IdentifierToken*) node->head));
        node = node->tail;
    }
    printf("\n");
    printTokenWithIndent(this->body, indent + 1);
}

uint8_t FuncToken::equals(Token* other) {
    FuncToken* otherFunc = (FuncToken*) other;

    return tokensEqual(this->name, otherFunc->name) &&
            allList2(this->args, otherFunc->args, tokensEqualVoid) &&
            tokensEqual(this->body, otherFunc->body);
}

Token* FuncToken::copy(CopyVisitor visitor, void* data) {
    IdentifierToken* name = (IdentifierToken*) visitor((Token*) this->name, data);
    List* args = copyTokenList(this->args, visitor, data);
    BlockToken* body = (BlockToken*) visitor(this->body, data);

    return (Token*) createFuncToken(this->location, name, args, body);
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

ReturnToken::~ReturnToken() {
    destroyToken(this->body);
}

TokenType ReturnToken::type() {
    return TOKEN_RETURN;
}

void ReturnToken::print(uint8_t indent) {
    printf("return:\n");
    printTokenWithIndent(this->body, indent + 1);
}

uint8_t ReturnToken::equals(Token* other) {
    return tokensEqual(this->body, ((ReturnToken*) other)->body);
}

Token* ReturnToken::copy(CopyVisitor visitor, void* data) {
    Token* copied = visitor(this->body, data);
    return (Token*) createReturnToken(this->location, copied);
}

Token* getReturnTokenBody(ReturnToken* token) {
    return token->body;
}

ReturnToken* createReturnToken(SrcLoc location, Token* body) {
    return new ReturnToken(location, body);
}

LabelToken::LabelToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

LabelToken::~LabelToken() {
    free((void*) this->name);
}

TokenType LabelToken::type() {
    return TOKEN_LABEL;
}

void LabelToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("label: %s \n", this->name);
}

uint8_t LabelToken::equals(Token* other) {
    return strcmp(this->name, ((LabelToken*) other)->name) == 0;
}

Token* LabelToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createLabelToken(newStr(this->name));
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

AbsJumpToken::~AbsJumpToken() {
    free((void*) this->label);
}

TokenType AbsJumpToken::type() {
    return TOKEN_ABS_JUMP;
}

void AbsJumpToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("absJump: %s\n", ((AbsJumpToken*) this)->label);
}

uint8_t AbsJumpToken::equals(Token* other) {
    return strcmp(this->label, ((AbsJumpToken*) other)->label) == 0;
}

Token* AbsJumpToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createAbsJumpToken(newStr(this->label));
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

CondJumpToken::~CondJumpToken() {
    destroyToken(this->condition);
    free((void*) this->label);
}

CondJumpToken::CondJumpToken(SrcLoc loc, Token* condition, const char* label, uint8_t when) :
    Token(loc), condition(condition), label(label), when(when) {}

TokenType CondJumpToken::type() {
    return TOKEN_COND_JUMP;
}

void CondJumpToken::print(uint8_t indent) {
    printf("condJump: %s, %i\n", this->label, this->when);
    printTokenWithIndent(this->condition, indent + 1);
}

uint8_t CondJumpToken::equals(Token* other) {
    CondJumpToken* otherJump = (CondJumpToken*) other;

    return tokensEqual(this->condition, otherJump->condition) &&
            strcmp(this->label, otherJump->label) == 0 &&
            this->when == otherJump->when;
}

Token* CondJumpToken::copy(CopyVisitor visitor, void* data) {
    Token* condition = visitor(this->condition, data);
    const char* label = newStr(this->label);
    return (Token*) createCondJumpToken(this->location, condition, label,
            this->when);
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

PushBuiltinToken::~PushBuiltinToken() {
    free((void*) this->name);
}

TokenType PushBuiltinToken::type() {
    return TOKEN_PUSH_BUILTIN;
}

void PushBuiltinToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("push_builtin: %s\n", this->name);
}

uint8_t PushBuiltinToken::equals(Token* other) {
    return strcmp(this->name, ((PushBuiltinToken*) other)->name) == 0;
}

Token* PushBuiltinToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createPushBuiltinToken(this->location, newStr(this->name));
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

void PushIntToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("push_int: %i\n", this->value);
}

uint8_t PushIntToken::equals(Token* other) {
    return this->value == ((PushIntToken*) other)->value;
}

Token* PushIntToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createPushIntToken(this->location, this->value);
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

void CallOpToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("op_call: %i\n", this->arity);
}

uint8_t CallOpToken::equals(Token* other) {
    return this->arity == ((CallOpToken*) other)->arity;
}

Token* CallOpToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createCallOpToken(this->location, this->arity);
}

uint8_t getCallOpTokenArity(CallOpToken* token) {
    return token->arity;
}

CallOpToken* createCallOpToken(SrcLoc loc, uint8_t arity) {
    return new CallOpToken(loc, arity);
}

StoreToken::StoreToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

StoreToken::~StoreToken() {
    free((void*) this->name);
}

TokenType StoreToken::type() {
    return TOKEN_STORE;
}

void StoreToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("store: %s\n", this->name);
}

uint8_t StoreToken::equals(Token* other) {
    return strcmp(this->name, ((StoreToken*) other)->name) == 0;
}

Token* StoreToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createStoreToken(this->location, newStr(this->name));
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

void DupToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("dup\n");
}

uint8_t DupToken::equals(Token* other) {
    UNUSED(other);
    return 1;
}

Token* DupToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createDupToken(this->location);
}

DupToken* createDupToken(SrcLoc loc) {
    return new DupToken(loc);
}

PushToken::PushToken(SrcLoc location, Token* value) :
    Token(location), value(value) {}

PushToken::~PushToken() {
    destroyToken(this->value);
}

TokenType PushToken::type() {
    return TOKEN_PUSH;
}

void PushToken::print(uint8_t ident) {
    printf("push:\n");
    printTokenWithIndent(this->value, ident + 1);
}

uint8_t PushToken::equals(Token* other) {
    return tokensEqual(this->value, ((PushToken*) other)->value);
}

Token* PushToken::copy(CopyVisitor visitor, void* data) {
    return (Token*) createPushToken(this->location, visitor(this->value, data));
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

void Rot3Token::print(uint8_t indent) {
    UNUSED(indent);
    printf("rot3\n");
}

uint8_t Rot3Token::equals(Token* other) {
    UNUSED(other);
    return 1;
}

Token* Rot3Token::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createRot3Token(this->location);
}

Rot3Token* createRot3Token(SrcLoc location) {
    return new Rot3Token(location);
}

SwapToken::SwapToken(SrcLoc location) :
    Token(location) {}

TokenType SwapToken::type() {
    return TOKEN_SWAP;
}

void SwapToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("swap\n");
}

uint8_t SwapToken::equals(Token* other) {
    UNUSED(other);
    return 1;
}

Token* SwapToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createSwapToken(this->location);
}

SwapToken* createSwapToken(SrcLoc loc) {
    return new SwapToken(loc);
}

PopToken::PopToken(SrcLoc location) :
    Token(location) {}

TokenType PopToken::type() {
    return TOKEN_POP;
}

void PopToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("pop\n");
}

uint8_t PopToken::equals(Token* other) {
    UNUSED(other);
    return 1;
}

Token* PopToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createPopToken(this->location);
}

PopToken* createPopToken(SrcLoc loc) {
    return new PopToken(loc);
}

BuiltinToken::BuiltinToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

BuiltinToken::~BuiltinToken() {
    free((void*) this->name);
}

TokenType BuiltinToken::type() {
    return TOKEN_BUILTIN;
}

void BuiltinToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("builtin: %s\n", this->name);
}

uint8_t BuiltinToken::equals(Token* other) {
    return strcmp(this->name, ((BuiltinToken*) other)->name) == 0;
}

Token* BuiltinToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createBuiltinToken(this->location, newStr(this->name));
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

void CheckNoneToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("check_none\n");
}

uint8_t CheckNoneToken::equals(Token* other) {
    UNUSED(other);
    return 1;
}

Token* CheckNoneToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createCheckNoneToken(this->location);
}

CheckNoneToken* createCheckNoneToken(SrcLoc location) {
    return new CheckNoneToken(location);
}

NewFuncToken::NewFuncToken(SrcLoc location, const char* name) :
    Token(location), name(name) {}

NewFuncToken::~NewFuncToken() {
    free((void*) this->name);
}

TokenType NewFuncToken::type() {
    return TOKEN_NEW_FUNC;
}

void NewFuncToken::print(uint8_t indent) {
    UNUSED(indent);
    printf("new_func: %s\n", this->name);
}

uint8_t NewFuncToken::equals(Token* other) {
    return strcmp(this->name, ((NewFuncToken*) other)->name) == 0;
}

Token* NewFuncToken::copy(CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    return (Token*) createNewFuncToken(this->location, newStr(this->name));
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
    if(token != NULL) {
        delete token;
    }
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
        token->print(indent);
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
    return a->equals(b);
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
    return token->copy(visitor, data);
}
