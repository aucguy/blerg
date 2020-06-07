#ifndef EXECUTE_H_
#define EXECUTE_H_

#include <stdint.h>

#include "main/codegen.h"
#include "main/runtime.h"
#include "main/util.h"

/**
 * Every 'object' in the language is internally referred to as a 'thing.'
 * So there are IntThing, LiteralThing, FuncThing (results of defs) and
 * ObjectThing. Each thing is stored as a Thing struct followed by custom data
 * which is specified by its type (Int, Literal, Func, Object, etc).
 *
 * The following does not apply
 *
 * Additionally, there are currently some restrictions on which how code can
 * execute. This will later be removed.
 *
 * 1) Native code can only call blerg code. Passing a thing that does not
 * represent code defined in blerg to callFunction will result in an error.
 *
 * 2) Blerg code cannot call other blerg code. Attempting to call a function
 * that was defined in blerg will result in an error.
 */

Runtime* createRuntime();
void destroyRuntime(Runtime* runtime);

/**
 * Executes the given module. The function executes the bytecode at index 0
 * until the module returns. Instead of normally returning the value returned',
 * (the top of the stack), the function returns the scope wrapped in an
 * ObjectThing.
 *
 * @param runtime the runtime object
 * @param module the module whose global scope is to be determined. Execution
 *          begins at the beginning of the bytecode until the first
 *          return instruction.
 * @param error set to a nonzero value if an error occurs during execution.
 * @return the global scope as a ModuleThing, or NULL upon error.
 */
RetVal executeModule(Runtime* runtime, Module* module);

/**
 * Calls the given thing with the given arguments. Currently, partial
 * application is not supported, so the number of arguments supplied must match
 * the arity of the function.
 *
 * @param runtime the runtime object
 * @param thing the function thing to invoke
 * @param argNo the number of arguments
 * @param args an array of Thing* whose length is argNo
 * @param error set to a nonzero value if an error occurs during execution
 * @return the thing returned by the function
 */
RetVal callFunction(Runtime* runtime, Thing* func, uint32_t argNo,
        Thing** args);

#endif /* EXECUTE_H_ */
