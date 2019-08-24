#define IS_PARSE_IMPL 1
#include <stdlib.h>
#include <string.h>
#include "main/parse.h"

/**
 * Returns whether or not a string contains the given character.
 * Returns false if the character is null.
 */
int containsChar(const char* chars, char c) {
    return c != 0 && strchr(chars, c);
}

/**
 * constructs a integer token
 *
 * @param value the token's value
 * @return the newly created token
 */
IntToken* createIntToken(int value) {
    IntToken* token = (IntToken*) malloc(sizeof(IntToken));
    token->token.type = TOKEN_INT;
    token->value = value;
    return token;
}

/**
 * constructs a literal token
 *
 * @param value a unique reference to the token's value.
 * @return the newly created token
 */
LiteralToken* createLiteralToken(const char* value) {
    LiteralToken* token = (LiteralToken*) malloc(sizeof(LiteralToken));
    token->token.type = TOKEN_LITERAL;
    token->value = value;
    return token;
}

/**
 * constructs an identifier token
 *
 * @param value a unique reference to the token's name.
 * @return the newly created token
 */
IdentifierToken* createIdentifierToken(const char* value) {
    IdentifierToken* token = (IdentifierToken*) malloc(sizeof(IdentifierToken));
    token->token.type = TOKEN_IDENTIFIER;
    token->value = value;
    return token;
}

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
    token->token.type = TOKEN_BINARY_OP;
    token->op = op;
    token->left = left;
    token->right = right;
    return token;
}

/**
 * constructs a unary op token
 *
 * @param op a unique reference to the token's operation.
 * @param child a unique reference to the token's operand.
 * @return the newly created token
 */
UnaryOpToken* createUnaryOpToken(const char* op, Token* child) {
    UnaryOpToken* token = (UnaryOpToken*) malloc(sizeof(UnaryOpToken));
    token->token.type = TOKEN_UNARY_OP;
    token->op = op;
    token->child = child;
    return token;
}

/**
 * constructs an assignment token
 *
 * @param left a unique reference to the token's lvalue.
 * @param right a unique reference to the token's rvalue.
 */
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

/**
 * Gets the current character for the ParseState.
 */
char getChar(ParseState* state) {
    return state->src[state->index];
}

/**
 * Advances the current character for the ParseState by the given amount of
 * characters.
 */
void advance(ParseState* state, int amount) {
    state->index += amount;
}

/**
 * Advances the current character for the ParseState by one character.
 */
void advance(ParseState* state) {
    state->index++;
}

/**
 * Advances the current character as long as the current character is a given
 * character.
 */
void advanceWhile(ParseState* state, const char* chars) {
    while(containsChar(chars, getChar(state)) != NULL) {
        advance(state);
    }
}

/**
 * Advances the current character until after the whitespace.
 */
void skipWhitespace(ParseState* state) {
    advanceWhile(state, " \t\n\r");
}

/**
 * Copies part of a string into a new string.
 *
 * @param str the string to take a slice of
 * @param start the index of the start of the slice
 * @param end the index of the end of the slice
 * @return a string containing the characters from the indexes of 'start' to
 *      'end' in 'src'
 */
char* sliceStr(const char* str, int start, int end) {
    int len = sizeof(char) * (end - start);
    char* extracted = (char*) malloc(len + 1);
    memcpy(extracted, &str[start], len);
    extracted[len] = 0;
    return extracted;
}

/**
 * The parsing algorithm uses recursive descent. Each syntactic construct has
 * its own function. The function takes an argument of the parse state and
 * increments the index until after the token. The function returns the
 * construct it just parsed.
 *
 * For example, parseInt takes the parse state, moves its index until after
 * the integer at the current index and returns an integer token. Likewise,
 * parseLiteral and parseIdentifier do the same, but with literal and
 * identifer tokens. parseFactor parses factors, which is made out of integers,
 * literals and identifiers. So, parseFactor calls parseInt, parseLiteral and
 * parseIdentifier.
 *
 * The parsing functions return NULL on errors.
 */

const char* INT_CHARS = "0123456789";

/**
 * Parses an integer.
 *
 * <integer> ::= [0-9]+
 */
IntToken* parseInt(ParseState* state) {
    int start = state->index;
    advanceWhile(state, INT_CHARS);

    char* extracted = sliceStr(state->src, start, state->index);
    int value = atoi(extracted);
    free(extracted);

    return createIntToken(value);
}

/**
 * Parses a literal.
 *
 * <literal> ::= ' .* '
 */
LiteralToken* parseLiteral(ParseState* state) {
    advance(state); //skip the '
    int start = state->index;
    while(getChar(state) != '\'' && getChar(state) != 0) {
        advance(state);
    }

    //EOF reached without terminating the string
    if(getChar(state) != '\'') {
        return NULL;
    }

    const char* value = sliceStr(state->src, start, state->index);
    advance(state); //skip the '

    return createLiteralToken(value);
}

const char* IDENTIFIER_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

/**
 * Parses an identifier
 *
 * <identifier> ::= [a-zA-Z_]+
 */
IdentifierToken* parseIdentifier(ParseState* state) {
    int start = state->index;
    advanceWhile(state, IDENTIFIER_CHARS);

    return createIdentifierToken(sliceStr(state->src, start, state->index));
}

/**
 * Parses a factor.
 *
 * <factor> ::= <integer> | <literal> | <identifier> | ( <expression> )
 */
Token* parseFactor(ParseState* state) {
    skipWhitespace(state);
    char c = getChar(state);
    if(c == '(') {
        advance(state); //skip the (
        Token* token = parseExpression(state);
        skipWhitespace(state);
        if(getChar(state) != ')') {
            destroyToken(token);
            return NULL;
        }
        advance(state); //skip the )
        return token;
    } else if(containsChar(INT_CHARS, c) != NULL) {
        return (Token*) parseInt(state);
    } else if(containsChar(IDENTIFIER_CHARS, c) != NULL) {
        return (Token*) parseIdentifier(state);
    } else if(c == '\'') {
        return (Token*) parseLiteral(state);
    } else {
        return NULL; //error on unknown
    }
}

/**
 * Returns whether or not the next characters matches the given string.
 */
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


//'end' is not an operator; it signifies that the end of the array of operators
const int OP_LEVELS = 5;
const int OP_AMOUNT = 7;
const char* OP_DATA[OP_LEVELS][OP_AMOUNT] = {
        { "*", "/", "end" },
        { "+", "-", "end" },
        { "==", "!=", ">", ">=", "<", "<=", "end" },
        //the 'prefix' isn't an operator; it signifies that this level contains
        //prefix unary operators.
        { "prefix", "not", "end" },
        { "and", "or", "end" }
};

/**
 * Returns the operator for the given level.
 */
const char* getOp(ParseState* state, int level) {
    for(int i = 0; i < OP_AMOUNT; i++) {
        const char* checking = OP_DATA[level][i];
        //end the array of operators, exit the function
        if(strcmp(checking, "end") == 0) {
            break;
        }
        //information for other functions, skip it
        if(strcmp(checking, "prefix") == 0) {
            continue;
        }
        if(lookAhead(state, checking)) {
            return newStr(checking);
        }
    }
    return NULL;
}

/**
 * The following functions parse expressions.
 *
 * <expression0> ::= <factor> (* | / <factor>)*
 * <expression1> ::= <expression0> (+ | - <expression0>)*
 * <expression2> ::= <expression1> (== | != | > | >= | < | <= <expression1>)*
 * <expression3> ::= not* <expression2>
 * <expression4> ::= <expression3> (and | or <expression3)*
 *
 * The grammer is repetitive. For binary operations, the grammer is:
 *
 * <expressionX> ::= <expression(X-1)> (OP1 | OP2 ... <expression(X-1)>
 *
 * For prefix unary operations:
 *
 * <expressionX> ::= (OP1 | OP2 ..)* <expression(X-1)>
 *
 * All binary operation expression constructs are handled by parseBinaryOp and
 * all prefix unary operation expression constructs are handled by parseUnaryOp.
 * These two functions take a 'level' argument which refers to the number in the
 * constructs name. This 'level' is used by the function to look up the operation
 * names in the OP_DATA table. When a function needs to parse a subexpression,
 * it calls parseExpression, with the level one less than it was passed.
 */

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


/**
 * The official definition for unary ops is
 *
 * <expressionX> ::= (OP1 | OP2 ...)* <expression(X-1)>
 *
 * Despite this the implementation follows the definition
 *
 * <expressionX> ::= (OP1 | OP2 ...) <expressionX> | <expression(X-1)>
 *
 * Note that these two are equivalent.
 */
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

/**
 * Parses an assignment
 *
 * <assignment> ::= <expression> ; | <identifier> = <expression ;
 */
Token* parseAssignment(ParseState* state) {
    skipWhitespace(state);
    Token* left = parseExpression(state);
    if(left == NULL) {
        return NULL;
    }
    skipWhitespace(state);
    if(getChar(state) == ';') {
        advance(state); //skip the ;
        return left;
    } else if(getChar(state) == '=') {
        advance(state); //skip the =
        skipWhitespace(state);
        Token* right = parseExpression(state);
        if(right == NULL || left->type != TOKEN_IDENTIFIER || getChar(state) != ';') {
            free(left);
            return NULL;
        }
        advance(state); //skip the ;
        IdentifierToken* identifier = (IdentifierToken*) left;
        return (Token*) createAssignmentToken(identifier, right);
    } else {
        free(left);
        return NULL;
    }
}

/**
 * Parses a list of statements.
 *
 * @param state the ParseState
 * @param error set to 1 if there's an error. The return value cannot indicate
 *      whether or not there's an error since the function returns NULL on
 *      nonerror conditions
 * @return A list of the rest of the statements in this block. If there are no
 *      more statements, this returns NULL.
 */
List* parseBlockHelper(ParseState* state, int* error) {
    skipWhitespace(state);
    //end of block
    if(lookAhead(state, "end")) {
        //NULL represents an empty list
        return NULL;
    }
    //the EOF was reached before the end of the block, error
    if(getChar(state) == 0) {
        *error = 1;
        return NULL;
    }
    //not the end, parse the next statement
    Token* head = parseAssignment(state);
    if(head == NULL) {
        destroyToken(head);
        *error = 1;
        return NULL;
    }
    //parse the rest of the block recursively
    List* tail = parseBlockHelper(state, error);
    if(*error) {
        destroyToken(head);
        return NULL;
    }
    //adds the statement to the beginning of the rest of the statements
    return consList(head, tail);
}

/**
 * Parses a block.
 *
 * <block> ::= <assignment>* end
 *
 * The algorithm must be expressed using a stack as the first statements
 * must be passed to consList last so they are the first in the list.
 */
BlockToken* parseBlock(ParseState* state) {
    //parseBlockHelper sets this to 1 if there's an error instead of returning
    //NULL
    int error = 0;
    List* list = parseBlockHelper(state, &error);
    if(error) {
        return NULL;
    }
    return createBlockToken(list);
}

void destroyTokenVoid(void*);

/**
 * Frees a token's memory, it's data's memory and subtokens recursively
 */
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
