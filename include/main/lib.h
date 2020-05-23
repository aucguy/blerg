#ifndef LIB_H_
#define LIB_H_

#include <stdint.h>

#include "main/execute.h"
#include "main/thing.h"

RetVal libPrint(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libInput(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libAssert(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libToStr(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libToInt(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libTryCatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

#endif /* LIB_H_ */
