#ifndef TESTS_H_
#define TESTS_H_

#include <stdint.h>

#include "main/util.h"

uint8_t cmdArgc;
const char** cmdArgs;

/**
 * Runs all the tests and reports any errors.
 */
uint8_t runTests(uint8_t argc, const char* args[]);

/**
 * if the condition is false, the test fails with the given message
 */
#define assert(cond, msg) if(!(cond)) { return msg; }

SrcLoc nowhere;

#endif /* TESTS_H_ */
