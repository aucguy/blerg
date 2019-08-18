#define IS_PARSE_IMPL 1
#include <stdlib.h>
#include <string.h>
#include "main/parse.h"

int containsChar(const char* chars, char c) {
    return c != 0 && strchr(chars, c);
}

IntToken* createIntToken(int value) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token.type = TOKEN_INT;
    token->value = value;
    return token;
}

LiteralToken* createLiteralToken(const char* value) {
    LiteralToken* token = (LiteralToken*) malloc(sizeof(LiteralToken));
    token->token.type = TOKEN_LITERAL;
    token->value = value;
    return token;
}

IdentifierToken* createIdentifierToken(const char* value) {
    IdentifierToken* token = (IdentifierToken*) malloc(sizeof(IdentifierToken));
    token->token.type = TOKEN_IDENTIFIER;
    token->value = value;
    return token;
}

BinaryOpToken* createBinaryOpToken(const char* op, Token* left, Token* right) {
    BinaryOpToken* token = (BinaryOpToken*) malloc(sizeof(BinaryOpToken));
    token->token.type = TOKEN_BINARY_OP;
    token->op = op;
    token->left = left;
    token->right = right;
    return token;
}

UnaryOpToken* createUnaryOpToken(const char* op, Token* child) {
    UnaryOpToken* token = (UnaryOpToken*) malloc(sizeof(UnaryOpToken));
    token->token.type = TOKEN_UNARY_OP;
    token->op = op;
    token->child = child;
    return token;
}

AssignmentToken* createAssignmentToken(IdentifierToken* left, Token* right) {
    AssignmentToken* token = (AssignmentToken*) malloc(sizeof(AssignmentToken));
    token->token.type = TOKEN_ASSIGNMENT;
    token->left = left;
    token->right = right;
    return token;
}

BlockToken* createBlockToken(List* children) {
    BlockToken* token = (BlockToken*) malloc(sizeof(BlockToken));
    token->token.type = TOKEN_BLOCK;
    token->children = children;
    return token;
}

ParseState* createParseState(const char* src) {
    ParseState* state = (ParseState*) malloc(sizeof(ParseState));
    state->index = 0;
    state->src = src;
    return state;
}

char getChar(ParseState* state) {
    return state->src[state->index];
}

void advance(ParseState* state, int amount) {
    state->index += amount;
}

void advance(ParseState* state) {
    state->index++;
}

void advanceWhile(ParseState* state, const char* chars) {
    while(containsChar(chars, getChar(state)) != NULL) {
        advance(state);
    }
}

void skipWhitespace(ParseState* state) {
    advanceWhile(state, " \t\n\r");
}

char* sliceStr(const char* str, int start, int end) {
    int len = sizeof(char) * (end - start);
    char* extracted = (char*) malloc(len + 1);
    memcpy(extracted, &str[start], len);
    extracted[len] = 0;
    return extracted;
}

const char* INT_CHARS = "0123456789";

IntToken* parseInt(ParseState* state) {
    int start = state->index;
    advanceWhile(state, INT_CHARS);

    char* extracted = sliceStr(state->src, start, state->index);
    int value = atoi(extracted);
    free(extracted);

    return createIntToken(value);
}


LiteralToken* parseLiteral(ParseState* state) {
    advance(state); //skip the '
    int start = state->index;
    while(getChar(state) != '\'' && getChar(state) != 0) {
        advance(state);
    }

    if(getChar(state) != '\'') {
        return NULL;
    }

    const char* value = sliceStr(state->src, start, state->index);
    advance(state); //skip the '

    return createLiteralToken(value);
}

const char* IDENTIFIER_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

IdentifierToken* parseIdentifier(ParseState* state) {
    int start = state->index;
    advanceWhile(state, IDENTIFIER_CHARS);

    return createIdentifierToken(sliceStr(state->src, start, state->index));
}

Token* parseFactor(ParseState* state) {
    skipWhitespace(state);
    char c = getChar(state);
    if(c == '(') {
        advance(state);
        Token* token = parseExpression(state);
        skipWhitespace(state);
        if(getChar(state) != ')') {
            destroyToken(token);
            return NULL;
        }
        advance(state);
        return token;
    } else if(containsChar(INT_CHARS, c) != NULL) {
        return (Token*) parseInt(state);
    } else if(containsChar(IDENTIFIER_CHARS, c) != NULL) {
        return (Token*) parseIdentifier(state);
    } else if(c == '\'') {
        return (Token*) parseLiteral(state);
    } else {
        return NULL;
    }
}

int lookAhead(ParseState* state, const char* str) {
    int i = 0;
    while(1) {
        char a = str[i];
        char b = state->src[state->index + i];
        if(a == 0) {
            return 1;
        }
        if(b == 0 || a != b) {
            return 0;
        }
        i++;
    }
}

const int OP_LEVELS = 5;
const int OP_AMOUNT = 7;
const char* OP_DATA[OP_LEVELS][OP_AMOUNT] = {
        { "*", "/", "end" },
        { "+", "-", "end" },
        { "==", "!=", ">", ">=", "<", "<=", "end" },
        { "prefix", "not", "end" },
        { "and", "or", "end" }
};

const char* getOp(ParseState* state, int level) {
    for(int i = 0; i < OP_AMOUNT; i++) {
        const char* checking = OP_DATA[level][i];
        if(strcmp(checking, "end") == 0) {
            break;
        }
        if(strcmp(checking, "prefix") == 0) {
            continue;
        }
        if(lookAhead(state, checking)) {
            return newStr(checking);
        }
    }
    return NULL;
}

Token* parseExpression(ParseState* state, int level);

Token* parseBinaryOp(ParseState* state, int level) {
    Token* token = parseExpression(state, level - 1);
    if(token == NULL) {
        return NULL;
    }

    skipWhitespace(state);

    const char* op;
    while((op = getOp(state, level)) != NULL) {
        advance(state, strlen(op));
        Token* right = parseExpression(state, level - 1);
        if(right == NULL) {
            free((void*) op);
            destroyToken(token);
            return NULL;
        }
        token = (Token*) createBinaryOpToken(op, token, right);
        skipWhitespace(state);
    }

    return token;
}

Token* parsePrefixUnaryOp(ParseState* state, int level) {
    skipWhitespace(state);
    const char* op = getOp(state, level);
    if(op != NULL) {
        advance(state, strlen(op));
        Token* child = parseExpression(state, level);
        if(child == NULL) {
            free((void*) op);
            return NULL;
        }
        return (Token*) createUnaryOpToken(op, child);
    } else {
        return parseExpression(state, level - 1);
    }
}

Token* parseExpression(ParseState* state, int level) {
    if(level == -1) {
        return parseFactor(state);
    } else if(strcmp(OP_DATA[level][0], "prefix") == 0) {
        return parsePrefixUnaryOp(state, level);
    } else {
        return parseBinaryOp(state, level);
    }
}

Token* parseExpression(ParseState* state) {
    return parseExpression(state, OP_LEVELS - 1);
}

Token* parseAssignment(ParseState* state) {
    skipWhitespace(state);
    Token* left = parseExpression(state);
    if(left == NULL) {
        return NULL;
    }
    skipWhitespace(state);
    if(getChar(state) == ';') {
        advance(state);
        return left;
    } else if(getChar(state) == '=') {
        advance(state);
        skipWhitespace(state);
        Token* right = parseExpression(state);
        if(right == NULL || left->type != TOKEN_IDENTIFIER || getChar(state) != ';') {
            free(left);
            return NULL;
        }
        advance(state);
        IdentifierToken* identifier = (IdentifierToken*) left;
        return (Token*) createAssignmentToken(identifier, right);
    } else {
        free(left);
        return NULL;
    }
}

List* parseBlockHelper(ParseState* state, int* error) {
    skipWhitespace(state);
    if(lookAhead(state, "end")) {
        return NULL;
    }
    if(getChar(state) == 0) {
        *error = 1;
        return NULL;
    }
    Token* head = parseAssignment(state);
    if(head == NULL) {
        destroyToken(head);
        *error = 1;
        return NULL;
    }
    List* tail = parseBlockHelper(state, error);
    if(*error) {
        destroyToken(head);
        return NULL;
    }
    return consList(head, tail);
}

BlockToken* parseBlock(ParseState* state) {
    int error = 0;
    List* list = parseBlockHelper(state, &error);
    if(error) {
        return NULL;
    }
    return createBlockToken(list);
}

void destroyTokenVoid(void*);

void destroyToken(Token* token) {
    if(token == NULL) {
        return;
    }
    if(token->type == TOKEN_LITERAL) {
        free((void*) ((LiteralToken*) token)->value);
    } else if(token->type == TOKEN_IDENTIFIER) {
        free((void*) ((IdentifierToken*) token)->value);
    } else if(token->type == TOKEN_BINARY_OP) {
        BinaryOpToken* binaryOp = (BinaryOpToken*) token;
        free((void*) binaryOp->op);
        destroyToken(binaryOp->left);
        destroyToken(binaryOp->right);
    } else if(token->type == TOKEN_UNARY_OP) {
        UnaryOpToken* unaryOp = (UnaryOpToken*) token;
        free((void*) unaryOp->op);
        destroyToken(unaryOp->child);
    } else if(token->type == TOKEN_ASSIGNMENT) {
        AssignmentToken* assignment = (AssignmentToken*) token;
        destroyToken((Token*) assignment->left);
        destroyToken(assignment->right);
    } else if(token->type == TOKEN_BLOCK) {
        destroyList(((BlockToken*) token)->children, destroyTokenVoid);
    }
    free(token);
}

void destroyTokenVoid(void* token) {
    destroyToken((Token*) token);
}
