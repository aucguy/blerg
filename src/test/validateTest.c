#include <stdio.h>

#include "test/validateTest.h"
#include "test/tests.h"

#include "main/tokens.h"
#include "main/parse.h"
#include "main/util.h"
#include "main/validate.h"

FuncToken* createSimpleFunc(const char* name, int number) {
    return createFuncToken(createIdentifierToken(newStr(name)), NULL,
            createBlockToken(consList(createIntToken(number), NULL)));
}

const char* validateTestOnlyFuncsToplevel() {
    FuncToken* stmt1 = createSimpleFunc("func1", 1);
    FuncToken* stmt2 = createSimpleFunc("func2", 2);
    IntToken* stmt3 = createIntToken(3);
    FuncToken* stmt4 = createSimpleFunc("func4", 4);

    BlockToken* good = createBlockToken(consList(stmt1, consList(stmt2, NULL)));
    BlockToken* bad = createBlockToken(consList(stmt3, consList(stmt4, NULL)));

    assert(validateOnlyFuncsToplevel(good), "good validation failed");
    assert(!validateOnlyFuncsToplevel(bad), "bad validation succeeded");

    destroyToken((Token*) good);
    destroyToken((Token*) bad);

    return NULL;
}
