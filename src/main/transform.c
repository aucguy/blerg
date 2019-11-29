#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "main/util.h"
#include "main/transform.h"

/**
 * Used to create a unique name. This function takes an integer, concatenates
 * it with '$' and increments uniqueId.
 */
const char* uniqueName(int* uniqueId) {
    int length = snprintf(NULL, NULL, "$%d", *uniqueId);
    char* name = (char*) malloc(length + 1);
    sprintf(name, "$%d", *uniqueId);
    (*uniqueId)++;
    return name;
}

/**
 * Each of the following functions takes a token, and returns a list of tokens
 * that represent it using jumps. In this way, control structures are flattened.
 *
 * Also, any generated name should be one more than the highest generated name
 * that appears previous to the generated name's first appearance. In other
 * words, the generated names increase by one each time a new one appears.
 */

List* toJumpsList(List* list, int* uniqueId);

List* toJumpsBlock(BlockToken* token, int* uniqueId) {
    return toJumpsList(token->children, uniqueId);
}

/**
 * Converts an IfToken to a list of tokens using jump tokens.
 * For example, the code
 *
 * if a then
 *     b
 * elif c then
 *     d
 * else
 *     e
 * end
 *
 * becomes
 *
 * condJump a, $next1, 0
 * statement b
 * goto $end
 * label $next1
 * condJump c, $next2, 0
 * statement d
 * goto $end
 * label $next2
 * statement e
 * label $end
 *
 * Note that each condition check jumps to the label right before the next
 * condition check (nextLabel) and the end of each block jumps to the label at
 * the end of the output (endLabel).
 */
List* toJumpsIf(IfToken* token, int* uniqueId) {
    List* output = NULL;
    List* stmts = NULL;
    //computed lazily to preserve the names of label and conditions
    const char* endLabel = NULL;

    for(List* node = token->branches; node != NULL; node = node->tail) {
        IfBranch* branch = (IfBranch*) node->head;

        //jump to the next branch if the above condition is false
        const char* nextLabel = uniqueName(uniqueId);
        CondJumpToken* condJump = createCondJumpToken(
                copyToken(branch->condition),
                newStr(nextLabel), 0);
        stmts = consList(condJump, stmts);

        //if the condition is true, the block is executed
        output = toJumpsList(branch->block->children, uniqueId);
        stmts = prependReverseList(output, stmts);
        destroyShallowList(output);

        //jump to after the if statement
        if(endLabel == NULL) {
            endLabel = uniqueName(uniqueId);
        }
        stmts = consList(createAbsJumpToken(newStr(endLabel)), stmts);

        //indicate the next branch to jump to if the condition was false
        stmts = consList(createLabelToken(newStr(nextLabel)), stmts);

        free((void*) nextLabel);
    }

    //output the else branch
    if(token->elseBranch != NULL) {
        output = toJumpsBlock(token->elseBranch, uniqueId);
        stmts = prependReverseList(output, stmts);
        destroyShallowList(output);
    }

    if(endLabel == NULL) {
        endLabel = uniqueName(uniqueId);
    }

    //create the label all if branches jump to after execution
    stmts = consList(createLabelToken(newStr(endLabel)), stmts);

    free((void*) endLabel);
    List* ret = reverseList(stmts);
    destroyShallowList(stmts);
    return ret;
}

/**
 * Converts a while token to a list of tokens using jump tokens.
 * For example the code
 *
 * while a do
 *     b
 * end
 *
 * becomes
 *
 * label $startLabel
 * condJump a, $endLabel, 0
 * b
 * $absJump $startLabel
 * label $endLabel
 */
List* toJumpsWhile(WhileToken* token, int* uniqueId) {
    List* output = NULL;
    List* stmts = NULL;
    const char* startLabel = uniqueName(uniqueId);

    //beginning of the control structure; jumped to at the end of the loop
    stmts = consList(createLabelToken(newStr(startLabel)), stmts);

    //jump to the end if the condition is false
    const char* endLabel = uniqueName(uniqueId);
    CondJumpToken* condJump = createCondJumpToken(
            copyToken(token->condition),
            newStr(endLabel), 0);
    stmts = consList(condJump, stmts);

    //output the body
    output = toJumpsList(token->body->children, uniqueId);
    stmts = prependReverseList(output, stmts);
    destroyShallowList(output);

    //end of the loop
    stmts = consList(createAbsJumpToken(newStr(startLabel)), stmts);
    stmts = consList(createLabelToken(newStr(endLabel)), stmts);

    free((void*) startLabel);
    free((void*) endLabel);

    List* ret = reverseList(stmts);
    destroyShallowList(stmts);
    return ret;
}

/**
 * Converts a function token's contents from using control structures to using
 * jumps.
 */
FuncToken* toJumpsFunc(FuncToken* token, int* uniqueId) {
    //copy the arguments since each token holds a unique reference
    List* newArgs = NULL;
    List* oldArgs = token->args;
    while(oldArgs != NULL) {
        IdentifierToken* oldId = (IdentifierToken*) oldArgs->head;
        IdentifierToken* newId = createIdentifierToken(newStr(oldId->value));
        newArgs = consList((void*) newId, newArgs);
        oldArgs = oldArgs->tail;
    }

    FuncToken* ret = createFuncToken(
            createIdentifierToken(newStr(token->name->value)),
            reverseList(newArgs),
            createBlockToken(toJumpsBlock(token->body, uniqueId)));

    destroyShallowList(newArgs);
    return ret;
}

/**
 * Converts a list of tokens from using control structures to using jumps and
 * labels.
 *
 * The uniqueId is used to create a unique name for each generated label and
 * variable. It is modified each time a new name is created.
 */
List* toJumpsList(List* list, int* uniqueId) {
    List* stmts = NULL;

    for(; list != NULL; list = list->tail) {
        Token* token = (Token*) list->head;

        List* output;

        switch(token->type) {
        case TOKEN_INT:
        case TOKEN_LITERAL:
        case TOKEN_IDENTIFIER:
        case TOKEN_BINARY_OP:
        case TOKEN_UNARY_OP:
        case TOKEN_ASSIGNMENT:
        case TOKEN_RETURN:
            stmts = consList(copyToken(token), stmts);
            break;
        case TOKEN_BLOCK:
            output = toJumpsList(((BlockToken*) token)->children, uniqueId);
            stmts = prependReverseList(output, stmts);
            destroyShallowList(output);
            break;
        case TOKEN_IF:
            output = toJumpsIf((IfToken*) token, uniqueId);
            stmts = prependReverseList(output, stmts);
            destroyShallowList(output);
            break;
        case TOKEN_WHILE:
            output = toJumpsWhile((WhileToken*) token, uniqueId);
            stmts = prependReverseList(output, stmts);
            destroyShallowList(output);
            break;
        case TOKEN_FUNC:
            stmts = consList(toJumpsFunc((FuncToken*) token, uniqueId), stmts);
            break;
        }
    }

    List* ret = reverseList(stmts);
    destroyShallowList(stmts);
    return ret;
}

/**
 * Converts the module from using control structures to using jumps and labels.
 */
BlockToken* transformControlToJumps(BlockToken* module) {
    int uniqueId = 0;
    return createBlockToken(toJumpsBlock(module, &uniqueId));
}

BlockToken* transformModule(BlockToken* module) {
    return transformControlToJumps(module);
}
