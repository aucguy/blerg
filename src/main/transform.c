#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "main/util.h"
#include "main/transform.h"

/**
 * Used to create a unique name. This function takes an integer, concatenates
 * it with '$' and increments uniqueId.
 */
const char* uniqueName(uint8_t* uniqueId) {
    int length = snprintf(NULL, 0, "$%d", *uniqueId);
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

List* toJumpsList(List* list, uint8_t* uniqueId);

List* toJumpsBlock(BlockToken* token, uint8_t* uniqueId) {
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
List* toJumpsIf(IfToken* token, uint8_t* uniqueId) {
    List* output = NULL;
    List* stmts = NULL;
    //computed lazily to preserve the names of label and conditions
    const char* endLabel = NULL;

    for(List* node = token->branches; node != NULL; node = node->tail) {
        IfBranch* branch = (IfBranch*) node->head;

        //jump to the next branch if the above condition is false
        const char* nextLabel = uniqueName(uniqueId);
        CondJumpToken* condJump = createCondJumpToken(
                branch->condition->location,
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
List* toJumpsWhile(WhileToken* token, uint8_t* uniqueId) {
    List* output = NULL;
    List* stmts = NULL;
    const char* startLabel = uniqueName(uniqueId);

    //beginning of the control structure; jumped to at the end of the loop
    stmts = consList(createLabelToken(newStr(startLabel)), stmts);

    //jump to the end if the condition is false
    const char* endLabel = uniqueName(uniqueId);
    CondJumpToken* condJump = createCondJumpToken(
            token->condition->location,
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
FuncToken* toJumpsFunc(FuncToken* token, uint8_t* uniqueId) {
    //copy the arguments since each token holds a unique reference
    List* newArgs = NULL;
    List* oldArgs = token->args;
    while(oldArgs != NULL) {
        IdentifierToken* oldId = (IdentifierToken*) oldArgs->head;
        SrcLoc location = oldId->token.location;
        IdentifierToken* newId = createIdentifierToken(location, newStr(oldId->value));
        newArgs = consList((void*) newId, newArgs);
        oldArgs = oldArgs->tail;
    }

    FuncToken* ret = createFuncToken(token->token.location,
            createIdentifierToken(token->token.location, newStr(token->name->value)),
            reverseList(newArgs),
            createBlockToken(token->body->token.location,
                    toJumpsBlock(token->body, uniqueId)));

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
List* toJumpsList(List* list, uint8_t* uniqueId) {
    List* stmts = NULL;

    for(; list != NULL; list = list->tail) {
        Token* token = (Token*) list->head;

        List* output;

        switch(token->type) {
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_LITERAL:
        case TOKEN_IDENTIFIER:
        case TOKEN_TUPLE:
        case TOKEN_LIST:
            //this case shouldn't happen, but its here for future-proofing
        case TOKEN_CALL:
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
        case TOKEN_LABEL:
        case TOKEN_ABS_JUMP:
        case TOKEN_COND_JUMP:
            break; //shouldn't happen anyway
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
    uint8_t uniqueId = 0;
    SrcLoc location = module->token.location;
    return createBlockToken(location, toJumpsBlock(module, &uniqueId));
}

Token* transformListToCons(Token* token);

void* transformListToConsVoid(void* token) {
    return transformListToCons(token);
}

Token* listToConsCall(Token* token) {
    CallToken* call = (CallToken*) token;
    List* transformed = mapList(call->children, transformListToConsVoid);
    return (Token*) createCallToken(token->location, transformed);
}

Token* listToConsBinaryOp(Token* token) {
    BinaryOpToken* binaryOp = (BinaryOpToken*) token;
    const char* op = newStr(binaryOp->op);
    Token* left = transformListToCons(binaryOp->left);
    Token* right = transformListToCons(binaryOp->right);
    return (Token*) createBinaryOpToken(token->location, op, left, right);
}

Token* listToConsUnaryOp(Token* token) {
    UnaryOpToken* unaryOp = (UnaryOpToken*) token;
    const char* op = newStr(unaryOp->op);
    Token* child = transformListToCons(unaryOp->child);
    return (Token*) createUnaryOpToken(token->location, op, child);
}

Token* listToConsAssignment(Token* token) {
    AssignmentToken* assignment = (AssignmentToken*) token;
    IdentifierToken* left = (IdentifierToken*)
            transformListToCons((Token*) assignment->left);
    Token* right = transformListToCons(assignment->right);
    return (Token*) createAssignmentToken(token->location, left, right);
}

Token* listToConsBlock(Token* token) {
    BlockToken* call = (BlockToken*) token;
    List* transformed = mapList(call->children, transformListToConsVoid);
    return (Token*) createBlockToken(token->location, transformed);
}

IfBranch* listToConsBranch(IfBranch* branch) {
    Token* condition = transformListToCons(branch->condition);
    BlockToken* block = (BlockToken*) listToConsBlock((Token*) branch->block);
    return createIfBranch(condition, block);
}

void* listToConsBranchVoid(void* branch) {
    return listToConsBranch(branch);
}

Token* listToConsIf(Token* token) {
    IfToken* ifToken = (IfToken*) token;
    List* branches = mapList(ifToken->branches, listToConsBranchVoid);
    BlockToken* elseBranch = (BlockToken*)
            listToConsBlock((Token*) ifToken->elseBranch);
    return (Token*) createIfToken(token->location, branches, elseBranch);
}

Token* listToConsWhile(Token* token) {
    WhileToken* whileToken = (WhileToken*) token;
    Token* condition = transformListToCons(whileToken->condition);
    BlockToken* body = (BlockToken*)
            transformListToCons((Token*) whileToken->body);
    return (Token*) createWhileToken(token->location, condition, body);
}

Token* listToConsFunc(Token* token) {
    FuncToken* func = (FuncToken*) token;
    IdentifierToken* name = (IdentifierToken*) copyToken((Token*) func->name);
    List* args = mapList(func->args, transformListToConsVoid);
    BlockToken* body = (BlockToken*) listToConsBlock((Token*) func->body);
    return (Token*) createFuncToken(token->location, name, args, body);
}

Token* listToConsReturn(Token* token) {
    ReturnToken* returnToken = (ReturnToken*) token;
    Token* body = transformListToCons(returnToken->body);
    return (Token*) createReturnToken(token->location, body);
}

Token* listToConsListHelper(List* elements) {
    if(elements == NULL) {
        //TODO create a token that always resolves to none, instead of refering
        //to the global scope
        SrcLoc location;
        location.line = 0;
        location.column = 0;
        return (Token*) createIdentifierToken(location, newStr("none"));
    } else {
        const char* op = newStr("::");
        Token* left = transformListToCons(elements->head);
        Token* right = listToConsListHelper(elements->tail);
        return (Token*) createBinaryOpToken(left->location, op, left, right);
    }
}

Token* listToConsList(Token* token) {
    ListToken* list = (ListToken*) token;
    return listToConsListHelper(list->elements);
}

Token* transformListToCons(Token* token) {
    switch(token->type) {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    case TOKEN_LITERAL:
    case TOKEN_IDENTIFIER:
    case TOKEN_TUPLE:
    case TOKEN_LABEL:
    case TOKEN_ABS_JUMP:
    case TOKEN_COND_JUMP:
        return copyToken(token);
        break;
    case TOKEN_CALL:
        return listToConsCall(token);
    case TOKEN_BINARY_OP:
        return listToConsBinaryOp(token);
    case TOKEN_UNARY_OP:
        return listToConsUnaryOp(token);
    case TOKEN_ASSIGNMENT:
        return listToConsAssignment(token);
    case TOKEN_BLOCK:
        return listToConsBlock(token);
    case TOKEN_IF:
        return listToConsIf(token);
    case TOKEN_WHILE:
        return listToConsWhile(token);
    case TOKEN_FUNC:
        return listToConsFunc(token);
    case TOKEN_RETURN:
        return listToConsReturn(token);
    case TOKEN_LIST:
        return listToConsList(token);
    }
    return NULL;
}

BlockToken* transformModule(BlockToken* module1) {
    BlockToken* module2 = (BlockToken*) transformListToCons((Token*) module1);
    BlockToken* module3 = transformControlToJumps(module2);
    destroyToken((Token*) module2);
    return module3;
}
