#ifndef VALIDATE_H_
#define VALIDATE_H_

#include "main/tokens.h"

/**
 * Returns true if all the direct children are functions.
 * Currently, the language does not support other statements at the toplevel.
 * This restriction will later be removed.
 */
int validateOnlyFuncsToplevel(BlockToken* module);

/**
 * Returns true if there are no inner functions. Currently, the language does
 * not support inner functions. This restriction will later be removed.
 */
int validateNoInnerFuncs(BlockToken* module);

#endif /* VALIDATE_H_ */
