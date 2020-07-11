#include "main/runtime.h"
#include "main/thing.h"
#include "main/thing/cell.h"

CellThing::CellThing(Thing* value) :
    value(value) {}

CellThing::~CellThing() {}

RetVal CellThing::call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return callFail(runtime);
}

RetVal CellThing::dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity) {
    return symbolDispatch(runtime, self, args, arity);
}

ThingType CellThing::type() {
    return TYPE_CELL;
}

Thing* createCellThing(Runtime* runtime, Thing* value) {
    return createThing(runtime, new CellThing(value));
}

Thing* getCellValue(Thing* cell) {
    return ((CellThing*) cell)->value;
}

void setCellValue(Thing* cell, Thing* value) {
    ((CellThing*) cell)->value = value;
}
