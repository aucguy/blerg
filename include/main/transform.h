#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "main/tokens.h"

BlockToken* transformControlToJumps(BlockToken* module);
BlockToken* transformFlattenBlocks(BlockToken* module);

/**
 * Takes the AST from parseModule and turns it into a form suitable for
 * compileModule. This performs operations that make it easier for
 * compilation.
 */
BlockToken* transformModule(BlockToken* module);

#endif /* TRANSFORM_H_ */
