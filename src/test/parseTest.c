#include <stdlib.h>
#include <stdio.h>

#include "test/parseTest.h"
#include "main/parse.h"

const char* testParseIdentifier() {
    ParseState* state = createParseState("1234");
    IntToken* token = parseInt(state);
    assert(token->token.type == TOKEN_INT, "token type is not int")
    assert(token->value == 1234, "token value is not 1234")
    free(state);
    free(token);
    return NULL;
}
