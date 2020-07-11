#ifndef THING_CELL_H_
#define THING_CELL_H_

class CellThing : public Thing {
public:
    Thing* value;

    CellThing(Thing* value);
    ~CellThing();

    RetVal call(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    RetVal dispatch(Runtime* runtime, Thing* self, Thing** args, uint8_t arity);
    ThingType type();
};

#endif /* THING_CELL_H_ */
