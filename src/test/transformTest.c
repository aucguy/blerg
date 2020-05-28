#include <stdlib.h>
#include <stdint.h>

#include "main/parse.h"
#include "main/tokens.h"
#include "main/transform.h"
#include "main/util.h"

#include "test/transformTest.h"
#include "test/tests.h"

#include "test/defaultTokenLoc.h"

const char* transformTestControlToJumps() {
    char* error = NULL;
    Token* parsed = (Token*) parseModule("def f z do a = 1; while 0 do if 0 then b = g 2; else c = 3; end end d = 4; end", &error);
    Token* transformed1 = (Token*) transformControlToJumps((BlockToken*) parsed);
    Token* transformed2 = (Token*) transformFlattenBlocks((BlockToken*) transformed1);
    destroyToken(transformed1);

    List* args = consList(createIdentifierToken(newStr("z")), NULL);

    Token* stmts[12] = {
        (Token*) createAssignmentToken(createIdentifierToken(newStr("a")), (Token*) createIntToken(1)),
        (Token*) createLabelToken(newStr("$0")),
        (Token*) createCondJumpToken((Token*) createIntToken(0), newStr("$1"), 0),
        (Token*) createCondJumpToken((Token*) createIntToken(0), newStr("$2"), 0),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("b")), (Token*) createCallToken(
                consList(createIdentifierToken(newStr("g")),
                consList((Token*) createIntToken(2), NULL)))),
        (Token*) createAbsJumpToken(newStr("$3")),
        (Token*) createLabelToken(newStr("$2")),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("c")), (Token*) createIntToken(3)),
        (Token*) createLabelToken(newStr("$3")),
        (Token*) createAbsJumpToken(newStr("$0")),
        (Token*) createLabelToken(newStr("$1")),
        (Token*) createAssignmentToken(createIdentifierToken(newStr("d")), (Token*) createIntToken(4))
    };

    List* body = NULL;
    for(uint8_t i = 12; i != 0; i--) {
        body = consList(stmts[i - 1], body);
    }

    Token* expected = (Token*) createBlockToken(consList(createFuncToken(
            createIdentifierToken(newStr("f")), args, createBlockToken(body)), NULL));

    assert(tokensEqual(expected, transformed2), "transformation failed");

    destroyToken(expected);
    destroyToken(parsed);
    destroyToken(transformed2);

    return NULL;
}

const char* transformationTestObjectDesugar() {
    ParseState* state = createParseState("{a: b, c: d}");
    Token* parsed = parseFactor(state);
    Token* transformed = transformObjectDesugar(parsed);

    Token* key = (Token*) createIdentifierToken(newStr("a"));
    Token* value = (Token*) createIdentifierToken(newStr("b"));
    List* elements = consList(key, consList(value, NULL));
    Token* pair1 = (Token*) createTupleToken(elements);

    key = (Token*) createIdentifierToken(newStr("c"));
    value = (Token*) createIdentifierToken(newStr("d"));
    elements = consList(key, consList(value, NULL));
    Token* pair2 = (Token*) createTupleToken(elements);

    elements = consList(pair1, consList(pair2, NULL));
    Token* list = (Token*) createListToken(elements);

    Token* expected = (Token*) createUnaryOpToken(newStr("object"), list);

    assert(tokensEqual(expected, transformed), "transformation failed");

    destroyToken(parsed);
    destroyToken(transformed);
    destroyToken(expected);
    free(state);
    return NULL;
}
