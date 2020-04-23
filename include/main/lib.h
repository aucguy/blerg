#ifndef LIB_H_
#define LIB_H_

#include <stdint.h>

#include "main/execute.h"
#include "main/thing.h"

Thing* libPrint(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error);

Thing* libInput(Runtime* runtime, Thing* self, Thing** args, uint8_t arity,
        uint8_t* error);

#endif /* LIB_H_ */
