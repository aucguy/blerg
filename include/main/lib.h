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

RetVal libTuple(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libCons(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libHead(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libTail(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libUnpackCons(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libCreateSymbol(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libObject(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libCreateCell(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
RetVal libGetCell(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
RetVal libSetCell(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libImport(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);

RetVal libIsNone(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);


#endif /* LIB_H_ */
