#define IS_PARSE_IMPL 1
#include <stdlib.h>
#include <string.h>
#include "main/parse.h"

void destroyTokenVoid(void*);

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

IfBranch* createIfBranch(BlockToken* condition, BlockToken* block) {
    IfBranch* branch = (IfBranch*) malloc(sizeof(IfBranch));
    branch->condition = condition;
    branch->block = block;
    return branch;
}

IfToken* createIfToken(List* branches, BlockToken* elseBranch) {
    IfToken* token = (IfToken*) malloc(sizeof(IfToken));
    token->token.type = TOKEN_IF;
    token->branches = branches;
    token->elseBranch = elseBranch;
    return token;
}

WhileToken* createWhileToken(BlockToken* condition, BlockToken* body) {
    WhileToken* token = (WhileToken*) malloc(sizeof(WhileToken));
    token->token.type = TOKEN_WHILE;
    token->condition = condition;
    token->body = body;
    return token;
}

FuncToken* createFuncToken(IdentifierToken* name, List* args, BlockToken* body) {
    FuncToken* token = (FuncToken*) malloc(sizeof(FuncToken));
    token->token.type = TOKEN_FUNC;
    token->name = name;
    token->args = args;
    token->body = body;
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

const char* WHITESPACE_CHARS = " \t\n\r";

/**
 * Advances the current character until after the whitespace.
 */
void skipWhitespace(ParseState* state) {
    advanceWhile(state, WHITESPACE_CHARS);
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

/**
 * Returns whether or not the next characters matches any of the given strings.
 */
int lookAhead(ParseState* state, const char* strs[]) {
    for(int i = 0; strcmp(strs[i], "~") != 0; i++) {
        if(lookAhead(state, strs[i])) {
            return 1;
        }
    }
    return 0;
}


//'end' is not an operator; it signifies that the end of the array of operators
const int OP_LEVELS = 6;
const int OP_AMOUNT = 8;
const char* OP_DATA[OP_LEVELS][OP_AMOUNT] = {
        //' ' is the application operator. It sits invisible between a function
        //and its argument
        { " ", "end" },
        { "*", "/", "end" },
        { "+", "-", "end" },
        { "==", "!=", ">", ">=", "<", "<=", "end" },
        //the 'prefix' isn't an operator; it signifies that this level contains
        //prefix unary operators.
        { "prefix", "not", "end" },
        { "and", "or", "end" }
};

const char* KEYWORDS[] = { "def", "if", "then", "do", "elif", "else", "while",
        "end", "and", "or", "not", "~" };

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
        if(strcmp(checking, " ") == 0) {
            //the operator is application if the next character is a token.
            //skip the operator, but don't move the state forward
            int index = state->index;
            while(containsChar(WHITESPACE_CHARS, state->src[index])) {
                index++;
            }
            char c = state->src[index];
            if((c == '(' || c == '"' || containsChar(INT_CHARS, c) ||
                    containsChar(IDENTIFIER_CHARS, c)) &&
                    !lookAhead(state, KEYWORDS)) {
                return newStr(checking);
            }
        } else if(lookAhead(state, checking)) {
            return newStr(checking);
        }
    }
    return NULL;
}

/**
 * The following functions parse expressions.
 *
 * <expression0> ::= <factor> (<factor>)?
 * <expression1> ::= <expression0> (* | / <expression0>)*
 * <expression2> ::= <expression1> (+ | - <expression1>)*
 * <expression3> ::= <expression2> (== | != | > | >= | < | <= <expression2>)*
 * <expression4> ::= not* <expression3>
 * <expression5> ::= <expression4> (and | or <expression3)*
 *
 * The grammer is repetitive. For binary operations, the grammer is:
 *
 * <expressionX> ::= <expression(X-1)> (OP1 | OP2 ... <expression(X-1)>)*
 *
 * or
 *
 * <expressionX> ::= <expression(X-1)> (<expression(X-1)>)?
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
        if(strcmp(op, " ") != 0) {
            advance(state, strlen(op));
        }
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

//ending keywords for if condition blocks
const char* IF_COND_ENDS[] = { "then", "~" };
//ending keywords for if non-else blocks
const char* IF_BODY_ENDS[] = { "elif", "else", "end", "~" };
//ending keywords for if else blocks
const char* IF_ELSE_ENDS[] = { "end", "~" };

/**
 * Parses a list of if nonelse branches. This function does not consume the ending
 * keyword.
 *
 * @param state the ParseState
 * @param error set to 1 if there's an error. The return value cannot indicate
 *      whether or not there's an error since the function returns NULL on
 *      nonerror conditions
 * @return A list of the rest of non-else branches in this block. If there are no
 *      more non-else branches, this returns NULL.
 */
List* parseIfStmtBranches(ParseState* state, int* error) {
    BlockToken* condition = parseBlock(state, IF_COND_ENDS);
    if(condition == NULL || !lookAhead(state, "then")) {
        *error = 1;
        return NULL;
    }
    advance(state, strlen("then"));
    BlockToken* body = parseBlock(state, IF_BODY_ENDS);
    if(body == NULL) {
        *error = 1;
        return NULL;
    }
    IfBranch* head = createIfBranch(condition, body);
    List* tail;
    if(lookAhead(state, "elif")) {
        advance(state, strlen("elif"));
        //parse the rest of the branches
        List* tail = parseIfStmtBranches(state, error);
        if(*error) {
            return NULL;
        }
        return consList(head, tail);
    } else if(lookAhead(state, "else") || lookAhead(state, "end")) {
        //end of the branches
        //don't advance so parseIfStmt knows there is or is not an else branch
        tail = NULL;
    } else {
        *error = 1; //no terminating keyword
        return NULL;
    }
    return consList(head, tail);
}

/**
 * Parses an if statement.
 *
 * <if> ::= if <block> then <block> (elif <block> then <block>)* (else <block>)? end
 */
IfToken* parseIfStmt(ParseState* state) {
    skipWhitespace(state);
    //just in case
    if(!lookAhead(state, "if")) {
        return NULL;
    }
    advance(state, strlen("if"));
    //an error code is used because parseIfStmtBranches returns NULL when there
    //are no more branches, not for errors. Note that parseIfStmtBranches does
    //not consume elif / else / end.
    int error = 0;
    List* branches = parseIfStmtBranches(state, &error);
    if(error) {
        return NULL;
    }
    BlockToken* elseBranch;
    if(lookAhead(state, "else")) {
        advance(state, strlen("else"));
        elseBranch = parseBlock(state, IF_ELSE_ENDS);
    } else if(lookAhead(state, "end")) {
        advance(state, strlen("end"));
        elseBranch = NULL;
    } else {
        //shouldn't happen
        elseBranch = NULL;
    }
    return createIfToken(branches, elseBranch);
}

const char* WHILE_COND_ENDS[] = { "do", "~" };
const char* WHILE_BODY_ENDS[] = { "end", "~" };

WhileToken* parseWhileStmt(ParseState* state) {
    if(!lookAhead(state, "while")) {
        return NULL;
    }
    advance(state, strlen("while"));

    BlockToken* conditional = parseBlock(state, WHILE_COND_ENDS);
    if(conditional == NULL || !lookAhead(state, "do")) {
        return NULL;
    }
    advance(state, strlen("do"));

    BlockToken* body = parseBlock(state, WHILE_BODY_ENDS);
    if(body == NULL || !lookAhead(state, "end")) {
        return NULL;
    }
    advance(state, strlen("end"));

    return createWhileToken(conditional, body);
}

/**
 * Parses the arguments.
 *
 * @param state the ParseState
 * @param error set to 1 if an error occured
 * @return the rest of the arguments as a list of IdentifierTokens or NULL if
 *      there are no more arguments
 */
List* parseArgs(ParseState* state, int* error) {
    skipWhitespace(state);
    //end of the arguments
    if(lookAhead(state, "do")) {
        return NULL;
    }
    IdentifierToken* head = parseIdentifier(state);
    if(head == NULL) {
        *error = 1;
        return NULL;
    }
    //parse the rest
    List* tail = parseArgs(state, error);
    if(*error) {
        destroyToken((Token*) head);
        return NULL;
    }
    return consList(head, tail);
}

const char* FUNC_ENDS[] = { "end", "~" };

/**
 * Parses a function definition
 */
FuncToken* parseFuncStmt(ParseState* state) {
    //check just in case
    if(!lookAhead(state, "def")) {
        return NULL;
    }
    advance(state, strlen("def"));

    skipWhitespace(state);
    IdentifierToken* name = parseIdentifier(state);
    if(name == NULL) {
        return NULL;
    }

    int error = 0;
    List* args = parseArgs(state, &error);
    if(error || !lookAhead(state, "do")) {
        destroyToken((Token*) name);
        return NULL;
    }

    advance(state, strlen("do"));
    BlockToken* body = parseBlock(state, FUNC_ENDS);
    if(body == NULL) {
        destroyToken((Token*) name);
        destroyList(args, destroyTokenVoid);
    }
    return createFuncToken(name, args, body);
}

Token* parseStatement(ParseState* state) {
    skipWhitespace(state);
    if(lookAhead(state, "if")) {
        return (Token*) parseIfStmt(state);
    } else if(lookAhead(state, "while")) {
        return (Token*) parseWhileStmt(state);
    } else if(lookAhead(state, "def")) {
        return (Token*) parseFuncStmt(state);
    } else {
        return parseAssignment(state);
    }
}

/**
 * Parses a list of statements. This function does not consume the ending
 * keyword.
 *
 * @param state the ParseState
 * @param ends contains the keywords that may end the block. "~" must be the
 *          last string.
 * @param error set to 1 if there's an error. The return value cannot indicate
 *      whether or not there's an error since the function returns NULL on
 *      nonerror conditions
 * @return A list of the rest of the statements in this block. If there are no
 *      more statements, this returns NULL.
 */
List* parseBlockHelper(ParseState* state, const char* ends[], int* error) {
    skipWhitespace(state);
    //check for the end of the block
    if(lookAhead(state, ends)) {
        //NULL represents an empty list
        return NULL;
    }
    //the EOF was reached before the end of the block, error
    if(getChar(state) == 0) {
        *error = 1;
        return NULL;
    }
    //not the end, parse the next statement
    Token* head = parseStatement(state);
    if(head == NULL) {
        destroyToken(head);
        *error = 1;
        return NULL;
    }
    //parse the rest of the block recursively
    List* tail = parseBlockHelper(state, ends, error);
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
 * <block> ::= <assignment>* END1 | END 2 ...
 *
 * The algorithm must be expressed using a stack as the first statements
 * must be passed to consList last so they are the first in the list.
 *
 * This function does not consume the ending keyword.
 *
 * @param state the ParseState
 * @param ends contains the keywords that may end the block. "~" must be the
 *          last string.
 * @return The resultant block token
 */
BlockToken* parseBlock(ParseState* state, const char* ends[]) {
    //parseBlockHelper sets this to 1 if there's an error instead of returning
    //NULL
    int error = 0;
    List* list = parseBlockHelper(state, ends, &error);
    if(error) {
        return NULL;
    }
    return createBlockToken(list);
}

void destroyTokenVoid(void*);
void destroyIfBranch(void*);

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
    } else if(token->type == TOKEN_IF) {
        IfToken* ifToken = (IfToken*) token;
        destroyList(ifToken->branches, destroyIfBranch);
        destroyToken((Token*) ifToken->elseBranch);
    } else if(token->type == TOKEN_WHILE) {
        WhileToken* whileToken = (WhileToken*) token;
        destroyToken((Token*) whileToken->condition);
        destroyToken((Token*) whileToken->body);
    } else if(token->type == TOKEN_FUNC) {
        FuncToken* funcToken = (FuncToken*) token;
        destroyToken((Token*) funcToken->name);
        destroyList(funcToken->args, destroyTokenVoid);
        destroyToken((Token*) funcToken->body);
    }
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
