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


void destroyIntToken(Token* self) {
    UNUSED(self);
}

void printIntToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("int: %i\n", ((IntToken*) self)->value);
}

uint8_t equalsIntToken(Token* self, Token* other) {
    return ((IntToken*) self)->value == ((IntToken*) other)->value;
}

Token* copyIntToken(Token* token, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    IntToken* intToken = (IntToken*) token;
    return (Token*) createIntToken(intToken->token.location, intToken->value);
}

Token INT_TYPE = {
        TOKEN_INT,
        destroyIntToken,
        printIntToken,
        equalsIntToken,
        copyIntToken,
        tokenInstanceFields
};

/**
 * constructs a integer token
 *
 * @param value the token's value
 * @return the newly created token
 */
IntToken* createIntToken(SrcLoc loc, int32_t value) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token = INT_TYPE;
    token->token.location = loc;
    token->value = value;
    return token;
}

void printFloatToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("float: %f\n", ((FloatToken*) self)->value);
}

uint8_t equalsFloatToken(Token* self, Token* other) {
    return ((FloatToken*) self)->value == ((FloatToken*) other)->value;
}

Token* copyFloatToken(Token* self, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    FloatToken* num = (FloatToken*) self;
    return (Token*) createFloatToken(self->location, num->value);
}

void destroyFloatToken(Token* self) {
    UNUSED(self);
}

Token FLOAT_TYPE = {
        TOKEN_FLOAT,
        destroyFloatToken,
        printFloatToken,
        equalsFloatToken,
        copyFloatToken,
        tokenInstanceFields
};

FloatToken* createFloatToken(SrcLoc location, float value) {
    FloatToken* token = (FloatToken*) malloc(sizeof(FloatToken));
    token->token = FLOAT_TYPE;
    token->token.location = location;
    token->value = value;
    return token;
}

void destroyLiteralToken(Token* self) {
    free((void*) ((LiteralToken*) self)->value);
}

void printLiteralToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("literal: %s\n", ((LiteralToken*) self)->value);
}

uint8_t equalsLiteralToken(Token* self, Token* other) {
    return strcmp(((LiteralToken*) self)->value, ((LiteralToken*) other)->value) == 0;
}

Token* copyLiteralToken(Token* token, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    LiteralToken* literal = (LiteralToken*) token;
    const char* copied = newStr(literal->value);
    return (Token*) createLiteralToken(literal->token.location, copied);
}

Token LITERAL_TYPE = {
        TOKEN_LITERAL,
        destroyLiteralToken,
        printLiteralToken,
        equalsLiteralToken,
        copyLiteralToken,
        tokenInstanceFields
};

/**
 * constructs a literal token
 *
 * @param value a unique reference to the token's value.
 * @return the newly created token
 */
LiteralToken* createLiteralToken(SrcLoc loc, const char* value) {
    LiteralToken* token = (LiteralToken*) malloc(sizeof(LiteralToken));
    token->token = LITERAL_TYPE;
    token->token.location = loc;
    token->value = value;
    return token;
}

void destroyIdentifierToken(Token* token) {
    free((void*) ((IdentifierToken*) token)->value);
}

void printIdentifierToken(Token* self, uint8_t indent) {
    UNUSED(indent);
    printf("identifier: %s\n", ((IdentifierToken*) self)->value);
}

uint8_t equalsIdentifierToken(Token* self, Token* other) {
    return strcmp(((IdentifierToken*) self)->value,
            ((IdentifierToken*) other)->value) == 0;
}

Token* copyIdentifierToken(Token* token, CopyVisitor visitor, void* data) {
    UNUSED(visitor);
    UNUSED(data);
    IdentifierToken* identifier = (IdentifierToken*) token;
    SrcLoc loc = identifier->token.location;
    return (Token*) createIdentifierToken(loc, newStr(identifier->value));
}

Token IDENTIFIER_TYPE = {
        TOKEN_IDENTIFIER,
        destroyIdentifierToken,
        printIdentifierToken,
        equalsIdentifierToken,
        copyIdentifierToken,
        tokenInstanceFields
};

/**
 * constructs an identifier token
 *
 * @param value a unique reference to the token's name.
 * @return the newly created token
 */
IdentifierToken* createIdentifierToken(SrcLoc loc, const char* value) {
    IdentifierToken* token = (IdentifierToken*) malloc(sizeof(IdentifierToken));
    token->token = IDENTIFIER_TYPE;
    token->token.location = loc;
    token->value = value;
    return token;
}

void destroyTupleToken(Token* self) {
    TupleToken* tuple = (TupleToken*) self;
    destroyList(tuple->elements, destroyTokenVoid);
}

void printTupleToken(Token* self, uint8_t indent) {
    printf("tuple:\n");
    List* elements = ((TupleToken*) self)->elements;
    uint8_t i = 0;

    while(elements != NULL) {
        printIndent(indent + 1);
        printf("%i:\n", i);
        printTokenWithIndent((Token*) elements->head, indent + 2);
        elements = elements->tail;
        i++;
    }
}

uint8_t equalsTupleToken(Token* self, Token* other) {
    TupleToken* tuple1 = (TupleToken*) self;
    TupleToken* tuple2 = (TupleToken*) other;
    return allList2(tuple1->elements, tuple2->elements, tokensEqualVoid);
}

Token* copyTupleToken(Token* self, CopyVisitor visitor, void* data) {
    TupleToken* tuple = (TupleToken*) self;
    List* copied = copyTokenList(tuple->elements, visitor, data);
    return (Token*) createTupleToken(tuple->token.location, copied);
}

Token TUPLE_TYPE = {
        TOKEN_TUPLE,
        destroyTupleToken,
        printTupleToken,
        equalsTupleToken,
        copyTupleToken,
        tokenInstanceFields
};

TupleToken* createTupleToken(SrcLoc location, List* elements) {
    TupleToken* tuple = (TupleToken*) malloc(sizeof(TupleToken));
    tuple->token = TUPLE_TYPE;
    tuple->token.location = location;
    tuple->elements = elements;
    return tuple;
}

void destroyListToken(Token* self) {
    ListToken* list = (ListToken*) self;
    destroyList(list->elements, destroyTokenVoid);
}

void printListToken(Token* self, uint8_t indent) {
    printf("list:\n");
    List* elements = ((ListToken*) self)->elements;
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

uint8_t equalsListToken(Token* self, Token* other) {
    ListToken* list1 = (ListToken*) self;
    ListToken* list2 = (ListToken*) other;
    return allList2(list1->elements, list2->elements, tokensEqualVoid);
}

Token* copyListToken(Token* self, CopyVisitor visitor, void* data) {
    TupleToken* list = (TupleToken*) self;
    List* copied = copyTokenList(list->elements, visitor, data);
    return (Token*) createListToken(list->token.location, copied);
}

Token LIST_TYPE = {
        TOKEN_LIST,
        destroyListToken,
        printListToken,
        equalsListToken,
        copyListToken,
        tokenInstanceFields
};

ListToken* createListToken(SrcLoc location, List* elements) {
    ListToken* list = (ListToken*) malloc(sizeof(ListToken));
    list->token = LIST_TYPE;
    list->token.location = location;
    list->elements = elements;
    return list;
}

ObjectPair* createObjectPair(Token* key, Token* value) {
    ObjectPair* pair = (ObjectPair*) malloc(sizeof(ObjectPair));
    pair->key = key;
    pair->value = value;
    return pair;
}

void destroyObjectToken(Token* self) {
    ObjectToken* object = (ObjectToken*) self;
    List* elements = object->elements;

    while(elements != NULL) {
        ObjectPair* pair = (ObjectPair*) elements->head;
        destroyToken(pair->key);
        destroyToken(pair->value);
        free(pair);
        elements = elements->tail;
    }
    destroyShallowList(object->elements);
}

void printObjectToken(Token* self, uint8_t indent) {
    ObjectToken* object = (ObjectToken*) self;
    printf("object:\n");

    List* elements = object->elements;
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

uint8_t equalsObjectToken(Token* self, Token* other) {
    ObjectToken* object1 = (ObjectToken*) self;
    ObjectToken* object2 = (ObjectToken*) other;

    List* elements1 = object1->elements;
    List* elements2 = object2->elements;

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

Token* copyObjectToken(Token* self, CopyVisitor visitor, void* data) {
    ObjectToken* object = (ObjectToken*) self;
    List* copied = copyObjectPairs(object->elements, visitor, data);
    return (Token*) createObjectToken(self->location, copied);
}

Token OBJECT_TYPE = {
        TOKEN_OBJECT,
        destroyObjectToken,
        printObjectToken,
        equalsObjectToken,
        copyObjectToken,
        tokenInstanceFields
};

ObjectToken* createObjectToken(SrcLoc location, List* elements) {
    ObjectToken* object = (ObjectToken*) malloc(sizeof(ObjectToken));
    object->token.location = location;
    object->token = OBJECT_TYPE;
    object->elements = elements;
    return object;
}

void destroyCallToken(Token* self) {
    List* children = ((CallToken*) self)->children;
    while(children != NULL) {
        destroyToken((Token*) children->head);
        children = children->tail;
    }
    destroyShallowList(((CallToken*) self)->children);
}

void printCallToken(Token* self, uint8_t indent) {
    printf("call:\n");
    List* children = ((CallToken*) self)->children;
    while(children != NULL) {
        printTokenWithIndent((Token*) children->head, indent + 1);
        children = children->tail;
    }
}
uint8_t equalsCallToken(Token* self, Token* other) {
    List* childrenA = ((CallToken*) self)->children;
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

Token* copyCallToken(Token* self, CopyVisitor visitor, void* data) {
    CallToken* call = (CallToken*) self;
    List* children = copyTokenList(call->children, visitor, data);
    return (Token*) createCallToken(call->token.location, children);
}

Token CALL_TYPE = {
        TOKEN_CALL,
        destroyCallToken,
        printCallToken,
        equalsCallToken,
        copyCallToken,
        tokenInstanceFields
};

CallToken* createCallToken(SrcLoc location, List* children) {
    CallToken* token = (CallToken*) malloc(sizeof(CallToken));
    token->token = CALL_TYPE;
    token->token.location = location;
    token->children = children;
    return token;
}

void destroyBinaryOpToken(Token* self) {
    BinaryOpToken* binaryOp = (BinaryOpToken*) self;
    free((void*) binaryOp->op);
    destroyToken(binaryOp->left);
    destroyToken(binaryOp->right);
}

void printBinaryOpToken(Token* self, uint8_t indent) {
    BinaryOpToken* binaryOp = (BinaryOpToken*) self;
    printf("binaryOp: '%s'\n", binaryOp->op);
    printTokenWithIndent(binaryOp->left, indent + 1);
    printTokenWithIndent(binaryOp->right, indent + 1);
}

uint8_t equalsBinaryOpToken(Token* self, Token* other) {
    BinaryOpToken* selfBinOp = (BinaryOpToken*) self;
    BinaryOpToken* otherBinOp = (BinaryOpToken*) other;
    return strcmp(selfBinOp->op, otherBinOp->op) == 0 &&
            tokensEqual(selfBinOp->left, otherBinOp->left) &&
            tokensEqual(selfBinOp->right, otherBinOp->right);
}

Token* copyBinaryOpToken(Token* self, CopyVisitor visitor, void* data) {
    BinaryOpToken* binOp = (BinaryOpToken*) self;
    const char* op = newStr(binOp->op);
    Token* left = visitor(binOp->left, data);
    Token* right = visitor(binOp->right, data);
    return (Token*) createBinaryOpToken(binOp->token.location, op, left, right);
}

Token BINARY_OP_TYPE = {
        TOKEN_BINARY_OP,
        destroyBinaryOpToken,
        printBinaryOpToken,
        equalsBinaryOpToken,
        copyBinaryOpToken,
        tokenInstanceFields
};

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
    token->token = BINARY_OP_TYPE;
    token->token.location = loc;
    token->op = op;
    token->left = left;
    token->right = right;
    return token;
}

void destroyUnaryOpToken(Token* self) {
    UnaryOpToken* unaryOp = (UnaryOpToken*) self;
    free((void*) unaryOp->op);
    destroyToken(unaryOp->child);
}

void printUnaryOpToken(Token* self, uint8_t indent) {
    UnaryOpToken* unaryOp = (UnaryOpToken*) self;
    printf("unaryOp: %s\n", unaryOp->op);
    printTokenWithIndent(unaryOp->child, indent + 1);
}

uint8_t equalsUnaryOpToken(Token* self, Token* other) {
    UnaryOpToken* selfUnOp = (UnaryOpToken*) self;
    UnaryOpToken* otherUnOp = (UnaryOpToken*) other;
    return strcmp(selfUnOp->op, otherUnOp->op) == 0 &&
            tokensEqual(selfUnOp->child, otherUnOp->child);
}

Token* copyUnaryOpToken(Token* self, CopyVisitor visitor, void* data) {
    UnaryOpToken* unOp = (UnaryOpToken*) self;
    return (Token*) createUnaryOpToken(self->location, newStr(unOp->op),
            visitor(unOp->child, data));
}

Token UNARY_OP_TYPE = {
        TOKEN_UNARY_OP,
        destroyUnaryOpToken,
        printUnaryOpToken,
        equalsUnaryOpToken,
        copyUnaryOpToken,
        tokenInstanceFields
};

/**
 * constructs a unary op token
 *
 * @param op a unique reference to the token's operation.
 * @param child a unique reference to the token's operand.
 * @return the newly created token
 */
UnaryOpToken* createUnaryOpToken(SrcLoc loc, const char* op, Token* child) {
    UnaryOpToken* token = (UnaryOpToken*) malloc(sizeof(UnaryOpToken));
    token->token = UNARY_OP_TYPE;
    token->token.location = loc;
    token->op = op;
    token->child = child;
    return token;
}

void destroyAssignmentToken(Token* self) {
    AssignmentToken* assignment = (AssignmentToken*) self;
    destroyToken((Token*) assignment->left);
    destroyToken(assignment->right);
}

void printAssignmentToken(Token* self, uint8_t indent) {
    AssignmentToken* assignment = (AssignmentToken*) self;
    printf("assignment:\n");

    printIndent(indent + 1);
    printf("lvalue:\n");
    printTokenWithIndent(assignment->left, indent + 2);

    printIndent(indent + 1);
    printf("rvalue:\n");
    printTokenWithIndent(assignment->right, indent + 2);
}

uint8_t equalsAssignmentToken(Token* self, Token* other) {
    AssignmentToken* selfAssign = (AssignmentToken*) self;
    AssignmentToken* otherAssign = (AssignmentToken*) other;
    return tokensEqual((Token*) selfAssign->left, (Token*) otherAssign->left) &&
            tokensEqual(selfAssign->right, otherAssign->right);
}

Token* copyAssignmentToken(Token* self, CopyVisitor visitor, void* data) {
    AssignmentToken* assign = (AssignmentToken*) self;
    Token* left = visitor((Token*) assign->left, data);
    Token* right = visitor(assign->right, data);
    return (Token*) createAssignmentToken(self->location, left, right);
}

Token ASSIGNMENT_TYPE = {
        TOKEN_ASSIGNMENT,
        destroyAssignmentToken,
        printAssignmentToken,
        equalsAssignmentToken,
        copyAssignmentToken,
        tokenInstanceFields
};

/**
 * constructs an assignment token
 *
 * @param left a unique reference to the token's lvalue.
 * @param right a unique reference to the token's rvalue.
 */
AssignmentToken* createAssignmentToken(SrcLoc loc, Token* left, Token* right) {
    AssignmentToken* token = (AssignmentToken*) malloc(sizeof(AssignmentToken));
    token->token = ASSIGNMENT_TYPE;
    token->token.location = loc;
    token->left = left;
    token->right = right;
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
    SrcLoc location = self->location;
    return (Token*) createBlockToken(location, copied);
}

Token BLOCK_TYPE = {
        TOKEN_BLOCK,
        destroyBlockToken,
        printBlockToken,
        equalsBlockToken,
        copyBlockToken,
        tokenInstanceFields
};

BlockToken* createBlockToken(SrcLoc location, List* children) {
    BlockToken* token = (BlockToken*) malloc(sizeof(BlockToken));
    token->token = BLOCK_TYPE;
    token->token.location = location;
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
    return (Token*) createIfToken(self->location, branches,
            (BlockToken*) elseBranch);
}

Token IF_TYPE = {
        TOKEN_IF,
        destroyIfToken,
        printIfToken,
        equalsIfToken,
        copyIfToken,
        tokenInstanceFields
};

IfToken* createIfToken(SrcLoc loc, List* branches, BlockToken* elseBranch) {
    IfToken* token = (IfToken*) malloc(sizeof(IfToken));
    token->token = IF_TYPE;
    token->token.location = loc;
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
    return (Token*) createWhileToken(self->location, condition, body);
}

Token WHILE_TYPE = {
        TOKEN_WHILE,
        destroyWhileToken,
        printWhileToken,
        equalsWhileToken,
        copyWhileToken,
        tokenInstanceFields
};

WhileToken* createWhileToken(SrcLoc loc, Token* condition, BlockToken* body) {
    WhileToken* token = (WhileToken*) malloc(sizeof(WhileToken));
    token->token = WHILE_TYPE;
    token->token.location = loc;
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
    printf("func: %s", func->name->value);
    List* node = func->args;
    while(node != NULL) {
        printf(" %s", ((IdentifierToken*) node->head)->value);
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

    return (Token*) createFuncToken(self->location, name, args, body);
}

Token FUNC_TYPE = {
        TOKEN_FUNC,
        destroyFuncToken,
        printFuncToken,
        equalsFuncToken,
        copyFuncToken,
        tokenInstanceFields
};

FuncToken* createFuncToken(SrcLoc loc, IdentifierToken* name, List* args,
        BlockToken* body) {
    FuncToken* token = (FuncToken*) malloc(sizeof(FuncToken));
    token->token = FUNC_TYPE;
    token->token.location = loc;
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
    return (Token*) createReturnToken(self->location, copied);
}

Token RETURN_TYPE = {
        TOKEN_RETURN,
        destroyReturnToken,
        printReturnToken,
        equalsReturnToken,
        copyReturnToken,
        tokenInstanceFields
};

ReturnToken* createReturnToken(SrcLoc location, Token* body) {
    ReturnToken* token = (ReturnToken*) malloc(sizeof(ReturnToken));
    token->token = RETURN_TYPE;
    token->token.location = location;
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

Token LABEL_TYPE = {
        TOKEN_LABEL,
        destroyLabelToken,
        printLabelToken,
        equalsLabelToken,
        copyLabelToken,
        tokenInstanceFields
};

LabelToken* createLabelToken(const char* name) {
    LabelToken* token = (LabelToken*) malloc(sizeof(LabelToken));
    token->token = LABEL_TYPE;
    token->token.location.line = 0;
    token->token.location.column = 0;
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

Token ABS_JUMP_TYPE = {
        TOKEN_ABS_JUMP,
        destroyAbsJumpToken,
        printAbsJumpToken,
        equalsAbsJumpToken,
        copyAbsJumpToken,
        tokenInstanceFields
};

AbsJumpToken* createAbsJumpToken(const char* label) {
    AbsJumpToken* token = (AbsJumpToken*) malloc(sizeof(AbsJumpToken));
    token->token = ABS_JUMP_TYPE;
    token->token.location.line = 0;
    token->token.location.column = 0;
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
    return (Token*) createCondJumpToken(self->location, condition, label,
            jump->when);
}

Token COND_JUMP_TYPE = {
        TOKEN_COND_JUMP,
        destroyCondJumpToken,
        printCondJumpToken,
        equalsCondJumpToken,
        copyCondJumpToken,
        tokenInstanceFields
};

CondJumpToken* createCondJumpToken(SrcLoc loc, Token* cond, const char* label,
        uint8_t when) {
    CondJumpToken* token = (CondJumpToken*) malloc(sizeof(CondJumpToken));
    token->token = COND_JUMP_TYPE;
    token->token.location = loc;
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
    return (Token*) createPushBuiltinToken(self->location, newStr(builtin->name));
}

Token PUSH_BUILTIN_TYPE = {
        TOKEN_PUSH_BUILTIN,
        destroyPushBuiltinToken,
        printPushBuiltinToken,
        equalsPushBuiltinToken,
        copyPushBuiltinToken,
        tokenInstanceFields
};

PushBuiltinToken* createPushBuiltinToken(SrcLoc loc, const char* name) {
    PushBuiltinToken* builtin = (PushBuiltinToken*) malloc(sizeof(PushBuiltinToken));
    builtin->token = PUSH_BUILTIN_TYPE;
    builtin->token.location = loc;
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
    return (Token*) createPushIntToken(self->location, push->value);
}

Token PUSH_INT_TYPE = {
        TOKEN_PUSH_INT,
        destroyPushIntToken,
        printPushIntToken,
        equalsPushIntToken,
        copyPushIntToken,
        tokenInstanceFields
};

PushIntToken* createPushIntToken(SrcLoc loc, int32_t value) {
    PushIntToken* push = (PushIntToken*) malloc(sizeof(PushIntToken));
    push->token = PUSH_INT_TYPE;
    push->token.location = loc;
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
    return (Token*) createCallOpToken(self->location, call->arity);
}

Token CALL_OP_TYPE = {
        TOKEN_OP_CALL,
        destroyCallOpToken,
        printCallOpToken,
        equalsCallOpToken,
        copyCallOpToken,
        tokenInstanceFields
};

CallOpToken* createCallOpToken(SrcLoc loc, uint8_t arity) {
    CallOpToken* call = (CallOpToken*) malloc(sizeof(CallOpToken));
    call->token = CALL_OP_TYPE;
    call->token.location = loc;
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
    return (Token*) createStoreToken(self->location, newStr(store->name));
}

Token STORE_TYPE = {
        TOKEN_STORE,
        destroyStoreToken,
        printStoreToken,
        equalsStoreToken,
        copyStoreToken,
        tokenInstanceFields
};

StoreToken* createStoreToken(SrcLoc loc, const char* name) {
    StoreToken* store = (StoreToken*) malloc(sizeof(StoreToken));
    store->token = STORE_TYPE;
    store->token.location = loc;
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
    return (Token*) createDupToken(self->location);
}

Token DUP_TYPE = {
        TOKEN_DUP,
        destroyDupToken,
        printDupToken,
        equalsDupToken,
        copyDupToken,
        tokenInstanceFields
};

DupToken* createDupToken(SrcLoc loc) {
    DupToken* dup = (DupToken*) malloc(sizeof(DupToken));
    dup->token = DUP_TYPE;
    dup->token.location = loc;
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
    return (Token*) createPushToken(self->location, visitor(push->value, data));
}

Token PUSH_TYPE = {
        TOKEN_PUSH,
        destroyPushToken,
        printPushToken,
        equalsPushToken,
        copyPushToken,
        tokenInstanceFields
};

PushToken* createPushToken(SrcLoc loc, Token* value) {
    PushToken* push = (PushToken*) malloc(sizeof(PushToken));
    push->token = PUSH_TYPE;
    push->token.location = loc;
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
    return (Token*) createRot3Token(self->location);
}

Token ROT3_TYPE = {
        TOKEN_ROT3,
        destroyRot3Token,
        printRot3Token,
        equalsRot3Token,
        copyRot3Token,
        tokenInstanceFields
};

Rot3Token* createRot3Token(SrcLoc location) {
    Rot3Token* rot = (Rot3Token*) malloc(sizeof(Rot3Token));
    rot->token = ROT3_TYPE;
    rot->token.location = location;
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
    return (Token*) createBuiltinToken(self->location, newStr(builtin->name));
}

Token BUILTIN_TYPE = {
        TOKEN_BUILTIN,
        destroyBuiltinToken,
        printBuiltinToken,
        equalsBuiltinToken,
        copyBuiltinToken,
        tokenInstanceFields
};

BuiltinToken* createBuiltinToken(SrcLoc loc, const char* name) {
    BuiltinToken* builtin = (BuiltinToken*) malloc(sizeof(BuiltinToken));
    builtin->token = BUILTIN_TYPE;
    builtin->token.location = loc;
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
    return (Token*) createSwapToken(self->location);
}

Token SWAP_TYPE = {
        TOKEN_SWAP,
        destroySwapToken,
        printSwapToken,
        equalsSwapToken,
        copySwapToken,
        tokenInstanceFields
};

SwapToken* createSwapToken(SrcLoc loc) {
    SwapToken* swap = (SwapToken*) malloc(sizeof(SwapToken));
    swap->token = SWAP_TYPE;
    swap->token.location = loc;
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
    return (Token*) createPopToken(self->location);
}

Token POP_TYPE = {
        TOKEN_POP,
        destroyPopToken,
        printPopToken,
        equalsPopToken,
        copyPopToken,
        tokenInstanceFields
};

PopToken* createPopToken(SrcLoc loc) {
    PopToken* pop = (PopToken*) malloc(sizeof(PopToken));
    pop->token = POP_TYPE;
    pop->token.location = loc;
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
    return (Token*) createCheckNoneToken(self->location);
}

Token CHECK_NONE_TYPE = {
        TOKEN_CHECK_NONE,
        destroyCheckNoneToken,
        printCheckNoneToken,
        equalsCheckNoneToken,
        copyCheckNoneToken,
        tokenInstanceFields
};

CheckNoneToken* createCheckNoneToken(SrcLoc location) {
    CheckNoneToken* check = (CheckNoneToken*) malloc(sizeof(CheckNoneToken));
    check->token = CHECK_NONE_TYPE;
    check->token.location = location;
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
    return (Token*) createNewFuncToken(self->location, newStr(func->name));
}

Token NEW_FUNC_TYPE = {
        TOKEN_NEW_FUNC,
        destroyNewFuncToken,
        printNewFuncToken,
        equalsNewFuncToken,
        copyNewFuncToken,
        tokenInstanceFields
};

NewFuncToken* createNewFuncToken(SrcLoc location, const char* name) {
    NewFuncToken* func = (NewFuncToken*) malloc(sizeof(NewFuncToken));
    func->token = NEW_FUNC_TYPE;
    func->token.location = location;
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
    token->destroy(token);
    free(token);
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
    } else if(token->print == NULL) {
        printf("unknown\n");
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
    if(a == NULL || b == NULL || a->type != b->type ||
            a->equals == NULL || b->equals == NULL) {
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
