#define IS_PARSE_IMPL 1
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "main/tokens.h"
#include "main/parse.h"

//TODO fix memleaks for incorrect syntax
//TODO add have parseIdentifier check for keywords

BlockToken* parseBlock(ParseState*, const char**);

/**
 * Returns whether or not a string contains the given character.
 * Returns false if the character is null.
 */
uint8_t containsChar(const char* chars, char c) {
    return c != 0 && strchr(chars, c);
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
void advance(ParseState* state, uint32_t amount) {
    state->index += amount;
}

/**
 * Advances the current character for the ParseState by one character.
 */
/*void advance(ParseState* state) {
    state->index++;
}*/

/**
 * Advances the current character as long as the current character is a given
 * character.
 */
void advanceWhile(ParseState* state, const char* chars) {
    while(containsChar(chars, getChar(state)) != 0) {
        advance(state, 1);
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
char* sliceStr(const char* str, uint32_t start, uint32_t end) {
    uint32_t len = sizeof(char) * (end - start);
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
    uint32_t start = state->index;
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
    advance(state, 1); //skip the '
    uint32_t start = state->index;
    while(getChar(state) != '\'' && getChar(state) != 0) {
        advance(state, 1);
    }

    //EOF reached without terminating the string
    if(getChar(state) != '\'') {
        return NULL;
    }

    const char* value = sliceStr(state->src, start, state->index);
    advance(state, 1); //skip the '

    return createLiteralToken(value);
}

const char* IDENTIFIER_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

/**
 * Parses an identifier
 *
 * <identifier> ::= [a-zA-Z_]+
 */
IdentifierToken* parseIdentifier(ParseState* state) {
    uint32_t start = state->index;
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
        advance(state, 1); //skip the (
        Token* token = parseExpression(state);
        skipWhitespace(state);
        if(getChar(state) != ')') {
            destroyToken(token);
            return NULL;
        }
        advance(state, 1); //skip the )
        return token;
    } else if(containsChar(INT_CHARS, c) != 0) {
        return (Token*) parseInt(state);
    } else if(containsChar(IDENTIFIER_CHARS, c) != 0) {
        return (Token*) parseIdentifier(state);
    } else if(c == '\'') {
        return (Token*) parseLiteral(state);
    } else {
        return NULL; //error on unknown
    }
}

//'end' is not an operator; it signifies that the end of the array of operators
#define OP_LEVELS 5
#define OP_AMOUNT 8
const char* OP_DATA[OP_LEVELS][OP_AMOUNT] = {
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
 * Returns whether or not the next characters matches the given string. If the
 * string is "0" then the function returns if the end of the source has been
 * reached.
 */
uint32_t lookAhead(ParseState* state, const char* str) {
    uint32_t i = 0;
    if(strcmp(str, "0") == 0) {
        return getChar(state) == 0;
    }
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
 * If zero is present, the function will return true if the end of the source
 * has been reached.
 */
uint8_t lookAheadMulti(ParseState* state, const char* strs[]) {
    for(uint8_t i = 0; strcmp(strs[i], "~") != 0; i++) {
        if(lookAhead(state, strs[i])) {
            return 1;
        }
    }
    return 0;
}

uint8_t factorAhead(ParseState* state) {
    char c = getChar(state);
    return c == '(' || c == '\'' || containsChar(INT_CHARS, c) != 0 ||
            (containsChar(IDENTIFIER_CHARS, c) != 0 &&
                    !lookAheadMulti(state, KEYWORDS));
}

Token* parseCall(ParseState* state) {
    skipWhitespace(state);
    //the first token may just be by itself or be the function in the call
    Token* first = parseFactor(state);
    if(first == NULL) {
        return NULL;
    }

    skipWhitespace(state);
    //check if its a lone factor or a function call
    if(!factorAhead(state)) {
        return first;
    }

    //its a function call
    List* children = consList(first, NULL);
    while(factorAhead(state)) {
        children = consList(parseFactor(state), children);
        skipWhitespace(state);
    }

    List* reversed = reverseList(children);
    destroyShallowList(children);
    return (Token*) createCallToken(reversed);
}

/**
 * Returns the operator for the given level.
 */
const char* getOp(ParseState* state, uint8_t level) {
    for(uint8_t i = 0; i < OP_AMOUNT; i++) {
        const char* checking = OP_DATA[level - 1][i];
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

Token* parseExpressionWithLevel(ParseState* state, uint8_t level);

Token* parseBinaryOp(ParseState* state, uint8_t level) {
    Token* token = parseExpressionWithLevel(state, level - 1);
    if(token == NULL) {
        return NULL;
    }

    skipWhitespace(state);

    const char* op;
    while((op = getOp(state, level)) != NULL) {
        advance(state, strlen(op));
        Token* right = parseExpressionWithLevel(state, level - 1);
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
Token* parsePrefixUnaryOp(ParseState* state, uint8_t level) {
    skipWhitespace(state);
    const char* op = getOp(state, level);
    if(op != NULL) {
        advance(state, strlen(op));
        Token* child = parseExpressionWithLevel(state, level);
        if(child == NULL) {
            free((void*) op);
            return NULL;
        }
        return (Token*) createUnaryOpToken(op, child);
    } else {
        return parseExpressionWithLevel(state, level - 1);
    }
}

Token* parseExpressionWithLevel(ParseState* state, uint8_t level) {
    if(level == 0) {
        return parseCall(state);
    } else if(strcmp("prefix", OP_DATA[level - 1][0]) == 0){
        return parsePrefixUnaryOp(state, level);
    } else {
        return parseBinaryOp(state, level);
    }
}

Token* parseExpression(ParseState* state) {
    return parseExpressionWithLevel(state, OP_LEVELS);
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
        advance(state, 1); //skip the ;
        return left;
    } else if(getChar(state) == '=') {
        advance(state, 1); //skip the =
        skipWhitespace(state);
        Token* right = parseExpression(state);
        if(right == NULL || left->type != TOKEN_IDENTIFIER || getChar(state) != ';') {
            //TODO fix memleak
            free(left);
            return NULL;
        }
        advance(state, 1); //skip the ;
        IdentifierToken* identifier = (IdentifierToken*) left;
        return (Token*) createAssignmentToken(identifier, right);
    } else {
        destroyToken(left);
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
List* parseIfStmtBranches(ParseState* state, uint8_t* error) {
    Token* condition = parseExpression(state);
    if(condition == NULL || !lookAhead(state, "then")) {
        //TODO fix memleak
        *error = 1;
        return NULL;
    }
    advance(state, strlen("then"));
    BlockToken* body = parseBlock(state, IF_BODY_ENDS);
    if(body == NULL) {
        //TODO fix memleak
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
            //TODO fix memleak
            return NULL;
        }
        return consList(head, tail);
    } else if(lookAhead(state, "else") || lookAhead(state, "end")) {
        //end of the branches
        //don't advance so parseIfStmt knows there is or is not an else branch
        tail = NULL;
    } else {
        //TODO fix memleak
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
    uint8_t error = 0;
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
        //TODO exit and warn on this case
        elseBranch = NULL;
    }
    if(!lookAhead(state, "end")) {
        destroyList(branches, destroyIfBranch);
        destroyToken((Token*) elseBranch);
        return NULL;
    }
    advance(state, strlen("end"));
    return createIfToken(branches, elseBranch);
}

const char* WHILE_COND_ENDS[] = { "do", "~" };
const char* WHILE_BODY_ENDS[] = { "end", "~" };

WhileToken* parseWhileStmt(ParseState* state) {
    if(!lookAhead(state, "while")) {
        return NULL;
    }
    advance(state, strlen("while"));

    Token* conditional = parseExpression(state);
    if(conditional == NULL || !lookAhead(state, "do")) {
        //TODO fix memleak
        return NULL;
    }
    advance(state, strlen("do"));

    BlockToken* body = parseBlock(state, WHILE_BODY_ENDS);
    if(body == NULL || !lookAhead(state, "end")) {
        //TODO fix memleak
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
List* parseArgs(ParseState* state, uint8_t* error) {
    skipWhitespace(state);
    //end of the arguments
    if(lookAhead(state, "do")) {
        return NULL;
    }

    skipWhitespace(state);
    if(!containsChar(IDENTIFIER_CHARS, getChar(state))) {
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
        //TODO fix memleak
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

    uint8_t error = 0;
    List* args = parseArgs(state, &error);
    if(error || !lookAhead(state, "do")) {
        //TODO fix memleak?
        destroyToken((Token*) name);
        return NULL;
    }

    advance(state, strlen("do"));
    BlockToken* body = parseBlock(state, FUNC_ENDS);
    if(body == NULL || !lookAhead(state, "end")) {
        //TODO fix memleak?
        destroyToken((Token*) name);
        destroyList(args, destroyTokenVoid);
        return NULL;
    }
    advance(state, strlen("end"));
    return createFuncToken(name, args, body);
}

ReturnToken* parseReturnStmt(ParseState* state) {
    if(!lookAhead(state, "<-")) {
        return NULL;
    }
    advance(state, strlen("<-"));
    Token* body = parseExpression(state);
    if(body == NULL) {
        return NULL;
    }
    skipWhitespace(state);
    if(getChar(state) != ';') {
        destroyToken(body);
        return NULL;
    }
    advance(state, 1);
    return createReturnToken(body);
}

Token* parseStatement(ParseState* state) {
    skipWhitespace(state);
    if(lookAhead(state, "if")) {
        return (Token*) parseIfStmt(state);
    } else if(lookAhead(state, "while")) {
        return (Token*) parseWhileStmt(state);
    } else if(lookAhead(state, "def")) {
        return (Token*) parseFuncStmt(state);
    } else if(lookAhead(state, "<-")) {
        return (Token*) parseReturnStmt(state);
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
List* parseBlockHelper(ParseState* state, const char* ends[], uint8_t* error) {
    skipWhitespace(state);
    //check for the end of the block
    if(lookAheadMulti(state, ends)) {
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
    uint8_t error = 0;
    List* list = parseBlockHelper(state, ends, &error);
    if(error) {
        return NULL;
    }
    return createBlockToken(list);
}

const char* MODULE_ENDS[] = { "0", "~" };

BlockToken* parseModule(const char* src) {
    ParseState* state = createParseState(src);
    BlockToken* token = parseBlock(state, MODULE_ENDS);
    uint8_t ended = getChar(state) == 0;
    free(state);
    if(!ended) {
        if(token != NULL) {
            destroyToken((Token*) token);
        }
        return NULL;
    }
    return token;
}
