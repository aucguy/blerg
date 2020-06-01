#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "main/top.h"
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
    runTest("parseTestGreaterAndLessThan", parseTestGreaterAndLessThan(), &status);
    runTest("parseTestFloat", parseTestFloat(), &status);
    runTest("parseTestTuple", parseTestTuple(), &status);
    runTest("parseTestCons", parseTestCons(), &status);
    runTest("parseTestObject", parseTestObject(), &status);

    runTest("validateTestOnlyFuncsToplevel", validateTestOnlyFuncsToplevel(), &status);
    runTest("validateTestNoInnerFuncs", validateTestNoInnerFuncs(), &status);

    runTest("transformTestControlToJumps", transformTestControlToJumps(), &status);
    runTest("transformationTestObjectDesugar", transformationTestObjectDesugar(), &status);
    runTest("transformationTestDestructureTuple", transformationTestDestructureTuple(), &status);

    runTest("codegenTestSimple", codegenTestSimple(), &status);
    runTest("codegenTestJumps", codegenTestJumps(), &status);
    runTest("codegenTestLiteralUnaryOp", codegenTestLiteralUnaryOp(), &status);

    runTest("executeTestGlobalHasMainFunc", executeTestGlobalHasMainFunc(), &status);
    runTest("executeTestMainFuncReturns1", executeTestMainFuncReturns1(), &status);
    runTest("executeTestAddFunction", executeTestAddSubFunction(), &status);
    runTest("executeTestMathExpr", executeTestMathExpr(), &status);
    runTest("executeTestStrRet", executeTestStrRet(), &status);
    runTest("executeTestStrConcat", executeTestStrConcat(), &status);
    runTest("executeTestIntEq", executeTestIntEq(), &status);
    runTest("executeTestIntNotEq", executeTestIntNotEq(), &status);
    runTest("executeTestIntLessThan", executeTestIntLessThan(), &status);
    runTest("executeTestIntLessThanEq", executeTestIntLessThanEq(), &status);
    runTest("executeTestIntGreaterThan", executeTestIntGreaterThan(), &status);
    runTest("executeTestIntGreaterThanEq", executeTestIntGreaterThanEq(), &status);
    runTest("executeTestStrEq", executeTestStrEq(), &status);
    runTest("executeTestStrNotEq", executeTestStrNotEq(), &status);
    runTest("executeTestBoolAnd", executeTestBoolAnd(), &status);
    runTest("executeTestBoolOr", executeTestBoolOr(), &status);
    runTest("executeTestBoolNot", executeTestBoolNot(), &status);
    runTest("executeTestIfStmt", executeTestIfStmt(), &status);
    runTest("executeTestAssignment", executeTestAssignment(), &status);
    runTest("executeTestWhileLoop", executeTestWhileLoop(), &status);
    runTest("executeTestNativeFunc", executeTestNativeFunc(), &status);
    runTest("executeTestRecFunc", executeTestRecFunc(), &status);

    struct dirent* file;
    DIR* dir = opendir("blg_tests");
    if(dir != NULL) {
        while((file = readdir(dir)) != NULL) {
            if(strcmp(file->d_name, "..") != 0 && strcmp(file->d_name, ".") != 0) {
                size_t len = strlen("blg_tests/") + strlen(file->d_name) + 1;
                char* filename = malloc(sizeof(char) * len);
                strcpy(filename, "blg_tests/");
                strcat(filename, file->d_name);
                char* src = readFile(filename);

                initThing();
                ExecFuncIn in;
                in.runtime = createRuntime();
                in.src = src;
                in.name = "main";
                in.arity = 1;
                in.args = malloc(sizeof(Thing*) * in.arity);
                in.args[0] = in.runtime->noneThing;
                ExecFuncOut out = execFunc(in);
                if(out.errorMsg != NULL) {
                    status = 1;
                    printf("error in %s:\n%s\n", filename, out.errorMsg);
                }
                free(filename);
                free(src);
                cleanupExecFunc(in, out);
            }
        }
        closedir(dir);
    }

    if(status == 0) {
        printf("all tests succeeded\n");
    }
    return status;
}

SrcLoc nowhere = { 0, 0 };
