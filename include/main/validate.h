#ifndef VALIDATE_H_
#define VALIDATE_H_

#include <stdint.h>

#include "main/tokens.h"

/**
 * Returns true if all the direct children are functions.
 * Currently, the language does not support other statements at the toplevel.
 * This restriction will later be removed.
 */
uint8_t validateOnlyFuncsToplevel(BlockToken* module);

/**
 * Returns true if there are no inner functions. Currently, the language does
 * not support inner functions. This restriction will later be removed.
 */
uint8_t validateNoInnerFuncs(BlockToken* module);

/**
 * Checks that a module from parseModule is indeed valid. parseModule does not
 * reject all invalid modules.
 */
uint8_t validateModule(BlockToken* module);

#endif /* VALIDATE_H_ */
