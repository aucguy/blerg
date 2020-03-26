#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "main/tokens.h"
#include "main/util.h"

#define UNUSED(x) (void)(x)

uint8_t tokensEqualVoid(void* a, void* b);
uint8_t branchesEqual(void* a, void* b);

void printIndent(uint8_t indent) {
    for(uint8_t i = 0; i < indent; i++) {
        printf("    ");
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

Token* copyIntToken(Token* token) {
    return (Token*) createIntToken(((IntToken*) token)->value);
}

Token INT_TYPE = {
        TOKEN_INT,
        destroyIntToken,
        printIntToken,
        equalsIntToken,
        copyIntToken
};

/**
 * constructs a integer token
 *
 * @param value the token's value
 * @return the newly created token
 */
IntToken* createIntToken(int32_t value) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token = INT_TYPE;
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

Token* copyLiteralToken(Token* token) {
    return (Token*) createLiteralToken(newStr(((LiteralToken*) token)->value));
}

Token LITERAL_TYPE = {
        TOKEN_LITERAL,
        destroyLiteralToken,
        printLiteralToken,
        equalsLiteralToken,
        copyLiteralToken
};

/**
 * constructs a literal token
 *
 * @param value a unique reference to the token's value.
 * @return the newly created token
 */
LiteralToken* createLiteralToken(const char* value) {
    LiteralToken* token = (LiteralToken*) malloc(sizeof(LiteralToken));
    token->token = LITERAL_TYPE;
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

Token* copyIdentifierToken(Token* token) {
    return (Token*) createIdentifierToken(newStr(((IdentifierToken*) token)->value));
}

Token IDENTIFIER_TYPE = {
        TOKEN_IDENTIFIER,
        destroyIdentifierToken,
        printIdentifierToken,
        equalsIdentifierToken,
        copyIdentifierToken
};

/**
 * constructs an identifier token
 *
 * @param value a unique reference to the token's name.
 * @return the newly created token
 */
IdentifierToken* createIdentifierToken(const char* value) {
    IdentifierToken* token = (IdentifierToken*) malloc(sizeof(IdentifierToken));
    token->token = IDENTIFIER_TYPE;
    token->value = value;
    return token;
}

void destroyCallToken(Token* self) {
    List* children = ((CallToken*) self)->children;
    while(children != NULL) {
        destroyToken(children->head);
        children = children->tail;
    }
    destroyShallowList(((CallToken*) self)->children);
}

void printCallToken(Token* self, uint8_t indent) {
    printf("call:\n");
    List* children = ((CallToken*) self)->children;
    while(children != NULL) {
        printTokenWithIndent(children->head, indent + 1);
        children = children->tail;
    }
}
uint8_t equalsCallToken(Token* self, Token* other) {
    List* childrenA = ((CallToken*) self)->children;
    List* childrenB = ((CallToken*) other)->children;

    while(childrenA != NULL && childrenB != NULL) {
        if(!tokensEqual(childrenA->head, childrenB->head)) {
            return 0;
        }
        childrenA = childrenA->tail;
        childrenB = childrenB->tail;
    }

    return childrenA == NULL && childrenB == NULL;
}

List* copyTokenList(List* old) {
    if(old == NULL) {
        return NULL;
    } else {
        return consList(copyToken(old->head), copyTokenList(old->tail));
    }
}

Token* copyCallToken(Token* self) {
    List* children = copyTokenList(((CallToken*) self)->children);
    return (Token*) createCallToken(children);
}

Token CALL_TYPE = {
        TOKEN_CALL,
        destroyCallToken,
        printCallToken,
        equalsCallToken,
        copyCallToken
};

CallToken* createCallToken(List* children) {
    CallToken* token = (CallToken*) malloc(sizeof(CallToken));
    token->token = CALL_TYPE;
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

Token* copyBinaryOpToken(Token* self) {
    BinaryOpToken* binOp = (BinaryOpToken*) self;
    return (Token*) createBinaryOpToken(newStr(binOp->op),
            copyToken(binOp->left), copyToken(binOp->right));
}

Token BINARY_OP_TYPE = {
        TOKEN_BINARY_OP,
        destroyBinaryOpToken,
        printBinaryOpToken,
        equalsBinaryOpToken,
        copyBinaryOpToken
};

/**
 * constructs a binary operation token
 *
 * @param op a unique reference to the token's operation.
 * @param left a unique reference to the left operand.
 * @param right a unique reference to the right operand.
 * @return the newly created token
 */
BinaryOpToken* createBinaryOpToken(const char* op, Token* left, Token* right) {
    BinaryOpToken* token = (BinaryOpToken*) malloc(sizeof(BinaryOpToken));
    token->token = BINARY_OP_TYPE;
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

Token* copyUnaryOpToken(Token* self) {
    UnaryOpToken* unOp = (UnaryOpToken*) self;
    return (Token*) createUnaryOpToken(newStr(unOp->op), copyToken(unOp->child));
}

Token UNARY_OP_TYPE = {
        TOKEN_UNARY_OP,
        destroyUnaryOpToken,
        printUnaryOpToken,
        equalsUnaryOpToken,
        copyUnaryOpToken
};

/**
 * constructs a unary op token
 *
 * @param op a unique reference to the token's operation.
 * @param child a unique reference to the token's operand.
 * @return the newly created token
 */
UnaryOpToken* createUnaryOpToken(const char* op, Token* child) {
    UnaryOpToken* token = (UnaryOpToken*) malloc(sizeof(UnaryOpToken));
    token->token = UNARY_OP_TYPE;
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
    printf("assignment: %s\n", assignment->left->value);
    printTokenWithIndent(assignment->right, indent + 1);
}

uint8_t equalsAssignmentToken(Token* self, Token* other) {
    AssignmentToken* selfAssign = (AssignmentToken*) self;
    AssignmentToken* otherAssign = (AssignmentToken*) other;
    return tokensEqual((Token*) selfAssign->left, (Token*) otherAssign->left) &&
            tokensEqual(selfAssign->right, otherAssign->right);
}

Token* copyAssignmentToken(Token* self) {
    AssignmentToken* assign = (AssignmentToken*) self;
    return (Token*) createAssignmentToken(
            (IdentifierToken*) copyToken((Token*) assign->left),
            copyToken(assign->right));
}

Token ASSIGNMENT_TYPE = {
        TOKEN_ASSIGNMENT,
        destroyAssignmentToken,
        printAssignmentToken,
        equalsAssignmentToken,
        copyAssignmentToken
};

/**
 * constructs an assignment token
 *
 * @param left a unique reference to the token's lvalue.
 * @param right a unique reference to the token's rvalue.
 */
AssignmentToken* createAssignmentToken(IdentifierToken* left, Token* right) {
    AssignmentToken* token = (AssignmentToken*) malloc(sizeof(AssignmentToken));
    token->token = ASSIGNMENT_TYPE;
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

Token BLOCK_TYPE = {
        TOKEN_BLOCK,
        destroyBlockToken,
        printBlockToken,
        equalsBlockToken,
        NULL
};

BlockToken* createBlockToken(List* children) {
    BlockToken* token = (BlockToken*) malloc(sizeof(BlockToken));
    token->token = BLOCK_TYPE;
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

Token IF_TYPE = {
        TOKEN_IF,
        destroyIfToken,
        printIfToken,
        equalsIfToken,
        NULL
};

IfToken* createIfToken(List* branches, BlockToken* elseBranch) {
    IfToken* token = (IfToken*) malloc(sizeof(IfToken));
    token->token = IF_TYPE;
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


Token WHILE_TYPE = {
        TOKEN_WHILE,
        destroyWhileToken,
        printWhileToken,
        equalsWhileToken,
        NULL
};

WhileToken* createWhileToken(Token* condition, BlockToken* body) {
    WhileToken* token = (WhileToken*) malloc(sizeof(WhileToken));
    token->token = WHILE_TYPE;
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

Token FUNC_TYPE = {
        TOKEN_FUNC,
        destroyFuncToken,
        printFuncToken,
        equalsFuncToken,
        NULL
};

FuncToken* createFuncToken(IdentifierToken* name, List* args, BlockToken* body) {
    FuncToken* token = (FuncToken*) malloc(sizeof(FuncToken));
    token->token = FUNC_TYPE;
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

Token* copyReturnToken(Token* self) {
    ReturnToken* token = (ReturnToken*) self;
    return (Token*) createReturnToken(copyToken(token->body));
}

Token RETURN_TYPE = {
        TOKEN_RETURN,
        destroyReturnToken,
        printReturnToken,
        equalsReturnToken,
        copyReturnToken
};

ReturnToken* createReturnToken(Token* body) {
    ReturnToken* token = (ReturnToken*) malloc(sizeof(ReturnToken));
    token->token = RETURN_TYPE;
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

Token LABEL_TYPE = {
        TOKEN_LABEL,
        destroyLabelToken,
        printLabelToken,
        equalsLabelToken,
        NULL
};

LabelToken* createLabelToken(const char* name) {
    LabelToken* token = (LabelToken*) malloc(sizeof(LabelToken));
    token->token = LABEL_TYPE;
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

Token ABS_JUMP_TYPE = {
        TOKEN_ABS_JUMP,
        destroyAbsJumpToken,
        printAbsJumpToken,
        equalsAbsJumpToken,
        NULL
};

AbsJumpToken* createAbsJumpToken(const char* label) {
    AbsJumpToken* token = (AbsJumpToken*) malloc(sizeof(AbsJumpToken));
    token->token = ABS_JUMP_TYPE;
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

Token COND_JUMP_TYPE = {
        TOKEN_COND_JUMP,
        destroyCondJumpToken,
        printCondJumpToken,
        equalsCondJumpToken,
        NULL
};

CondJumpToken* createCondJumpToken(Token* cond, const char* label, uint8_t when) {
    CondJumpToken* token = (CondJumpToken*) malloc(sizeof(CondJumpToken));
    token->token = COND_JUMP_TYPE;
    token->condition = cond;
    token->label = label;
    token->when = when;
    return token;
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

Token* copyToken(Token* token) {
    return token->copy(token);
}
