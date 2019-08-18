#include <stdio.h>
#include "test/parseTest.h"

void runTest(const char* name, const char* msg, int* status) {
    if(msg != NULL) {
        printf("test '%s' failed: %s\n", name, msg);
        *status = 1;
    }
}

int runTests() {
    int status = 0;
    printf("running tests...\n");

    runTest("testParseInt", testParseInt(), &status);
    runTest("testParseLiteral", testParseLiteral(), &status);
    runTest("testParseIdentifier", testParseIdentifier(), &status);
    runTest("testParseExpression", testParseExpression(), &status);
    runTest("testParseFail", testParseFail(), &status);
    runTest("testParseAssignments", testParseAssignments(), &status);
    runTest("parseTestBlockWithoutEndFails", parseTestBlockWithoutEndFails(), &status);

    if(status == 0) {
        printf("all tests succeeded");
    }
    return status;
}
