#include <stdio.h>
#include <stdint.h>

#include "test/parseTest.h"
#include "test/validateTest.h"
#include "test/transformTest.h"
#include "test/codegenTest.h"
#include "test/executeTest.h"

void runTest(const char* name, const char* msg, uint8_t* status) {
    if(msg != NULL) {
        printf("test '%s' failed: %s\n", name, msg);
        *status = 1;
    }
}

uint8_t runTests() {
    uint8_t status = 0;
    printf("running tests...\n");

    runTest("testParseInt", testParseInt(), &status);
    runTest("testParseLiteral", testParseLiteral(), &status);
    runTest("testParseIdentifier", testParseIdentifier(), &status);
    runTest("testParseExpression", testParseExpression(), &status);
    runTest("testParseFail", testParseFail(), &status);
    runTest("testParseAssignments", testParseAssignments(), &status);
    runTest("parseTestBlockWithEndFails", parseTestBlockWithEndFails(), &status);
    runTest("parseTestIfStmt", parseTestIfStmt(), &status);
    runTest("parseTestWhileStmt", parseTestWhileStmt(), &status);
    runTest("parseTestFunc", parseTestFunc(), &status);
    runTest("parseTestFuncWithoutDo", parseTestFuncWithoutDo(), &status);

    runTest("validateTestOnlyFuncsToplevel", validateTestOnlyFuncsToplevel(), &status);
    runTest("validateTestNoInnerFuncs", validateTestNoInnerFuncs(), &status);

    runTest("transformTestControlToJumps", transformTestControlToJumps(), &status);

    runTest("codegenTestSimple", codegenTestSimple(), &status);
    runTest("codegenTestJumps", codegenTestJumps(), &status);
    runTest("codegenTestLiteralUnaryOp", codegenTestLiteralUnaryOp(), &status);

    runTest("executeTestGlobalHasMainFunc", executeTestGlobalHasMainFunc(), &status);
    runTest("executeTestMainFuncReturns1", executeTestMainFuncReturns1(), &status);
    runTest("executeTestAddFunction", executeTestAddSubFunction(), &status);
    runTest("executeTestMathExpr", executeTestMathExpr(), &status);
    runTest("executeTestStrRet", executeTestStrRet(), &status);
    runTest("executeTestStrConcat", executeTestStrConcat(), &status);

    if(status == 0) {
        printf("all tests succeeded");
    }
    return status;
}
