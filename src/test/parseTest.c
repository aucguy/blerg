#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test/parseTest.h"
#include "main/parse.h"

const char* testParseInt() {
    ParseState* state = createParseState("1234");
    IntToken* token = parseInt(state);
    assert(token->token.type == TOKEN_INT, "token type is not int")
    assert(token->value == 1234, "token value is not 1234")
    free(state);
    destroyToken(&token->token);
    return NULL;
}

const char* testParseLiteral() {
    ParseState* state = createParseState("'Hello World'");
    LiteralToken* token = parseLiteral(state);
    assert(token->token.type == TOKEN_LITERAL, "token type is not literal");
    assert(strcmp(token->value, "Hello World") == 0, "token value is not 'Hello World'");
    free(state);
    destroyToken(&token->token);
    return NULL;
}
