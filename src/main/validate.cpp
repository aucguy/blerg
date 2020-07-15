#include <stdlib.h>
#include <stdint.h>

#include "main/tokens.h"

#define UNUSED(x) (void)(x)

uint8_t validateOnlyFuncsToplevel(BlockToken* module) {
    if(getTokenType((Token*) module) != TOKEN_BLOCK) {
        return 0;
    }
    List* node = getBlockTokenChildren(module);
    while(node != NULL) {
        if(getTokenType(((Token*) node->head)) != TOKEN_FUNC) {
            return 0;
        }
        node = node->tail;
    }
    return 1;
}

uint8_t containsNoFuncsBranch(void*);
uint8_t containsNoFuncsVoid(void*);

/**
 * Returns true if the token and all of its subtokens are not functions.
 */
uint8_t containsNoFuncs(Token* token) {
    if(token == NULL) {
        return 1;
    } else if(getTokenType(token) == TOKEN_BLOCK) {
        List* children = getBlockTokenChildren((BlockToken*) token);
        return allList(children, containsNoFuncsVoid);
    } else if(getTokenType(token) == TOKEN_IF) {
        IfToken* ifToken = (IfToken*) token;
        return allList(getIfTokenBranches(ifToken), containsNoFuncsBranch) &&
                containsNoFuncs((Token*) getIfTokenElseBranch(ifToken));
    } else if(getTokenType(token) == TOKEN_WHILE) {
        WhileToken* whileToken = (WhileToken*) token;
        return containsNoFuncs((Token*) whileToken->condition) &&
                containsNoFuncs((Token*) whileToken->body);
    } else if(getTokenType(token) == TOKEN_FUNC) {
        return 0;
    } else {
        return 1;
    }
}

uint8_t containsNoFuncsVoid(void* token) {
    return containsNoFuncs((Token*) token);
}

/**
 * Returns true if the branch does not contain functions.
 */
uint8_t containsNoFuncsBranch(void* branch) {
    IfBranch* casted = (IfBranch*) branch;
    return containsNoFuncs((Token*) casted->condition) &&
            containsNoFuncs((Token*) casted->block);
}

/**
 * Returns true if the given token does not contain functions. It does not
 * matter if the token itself is not a function. This function only works on
 * function tokens.
 */
uint8_t noInnerFuncs(Token* token) {
    if(getTokenType(token) == TOKEN_FUNC) {
        if(!allList(getBlockTokenChildren(((FuncToken*) token)->body),
                (uint8_t (*)(void*)) containsNoFuncs)) {
            return 0;
        }
    }
    return 1;
}

/**
 * Returns true if the module contains no inner functions
 */
uint8_t validateNoInnerFuncs(BlockToken* module) {
    return allList(getBlockTokenChildren(module), (uint8_t (*)(void*)) noInnerFuncs);
}

uint8_t validateModule(BlockToken* module) {
    UNUSED(module);
    //the checks are going away soon
    return 1;
    //return validateOnlyFuncsToplevel(module) && validateNoInnerFuncs(module);
}
