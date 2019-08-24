#ifndef TESTS_H_
#define TESTS_H_

/**
 * Runs all the tests and reports any errors.
 */
int runTests();

/**
 * if the condition is false, the test fails with the given message
 */
#define assert(cond, msg) if(!(cond)) { return msg; }

#endif /* TESTS_H_ */
