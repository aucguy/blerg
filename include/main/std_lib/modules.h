#ifndef STD_LIB_MODULES_H_
#define STD_LIB_MODULES_H_

#include "main/thing.h"
#include "main/execute.h"

Thing* loadBuiltinModule(Runtime* runtime, const char* filename);
void destroyBuiltinModules();

#endif /* STD_LIB_MODULES_H_ */
