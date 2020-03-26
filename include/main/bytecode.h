#ifndef BYTECODE_H_
#define BYTECODE_H_

#include <stdint.h>

enum INSTRUCTIONS {
    //args: value (uint32)
    //stack: -> uin32
    //reads an integer from the bytecode and pushes its value onto the stack
    OP_PUSH_INT,
    //args: index (uint32)
    //stack: -> symbol
    //reads an integer from the bytecode, looks it up in the constant table,
    //and pushes the associated builtin value onto the stack.
    OP_PUSH_BUILTIN,
    OP_PUSH_LITERAL,
    //args:
    //stack: -> None
    //pushes the None object onto the stack
    OP_PUSH_NONE,
    //args: arity (uint32)
    //stack: f x_1 x_2 ... x_n -> y
    //pops each x and f off the stack, calls f with each x and then pushes the
    //returned value onto the stack.
    OP_CALL,
    //args:
    //stack: x ->
    //pops x off the stack and returns x. This is the value that is pushed onto
    //the stack in the OP_CALL instruction of the caller.
    OP_RETURN,
    //args: label (uint32)
    //stack: -> func
    //creates a new function which inherits the current scope is the current
    //scope and whose entry point is the label.
    OP_CREATE_FUNC,
    //args: name (uint32)
    //stack: -> value
    //Looks up the name in the constant table. The instruction then pushes the
    //local variable with that name in the local scope.
    OP_LOAD,
    //args: name (uint32)
    //stack: value ->
    //Looks up the name in the constant table. The instruction then assigns the
    //local variable with that name within the local scope.
    OP_STORE,
    //args: label (uint32)
    //Pops a value. If the value is truthy, the instruction jumps to the label.
    OP_COND_JUMP_TRUE,
    //args: label (uint32)
    //Pops a value. If the value is falsy, the instruction jumps to the label.
    OP_COND_JUMP_FALSE,
    //args: label ()
    //The instruction jumps to the label unconditionally.
    OP_ABS_JUMP,
    //This opcode performs no operations, but denotes the beginning of a
    //function. It is used to tell the runtime function object the function's
    //arity and the names to bind the arguments to. The format is
    //
    //<byte opcode = OP_DEF_FUNC> <uint8 argNum> (<uint32 argNameIndex>)+
    //
    //The opcode is followed by an integer operand that denotes the function's
    //arity. Following the arity is a variable number of integer operands whose
    //values correspond to the index of the argument names in the constant
    //table. The number of these arguments is equal to the functions arity
    //found in the first operand.
    //
    //For example, "def myFunc x y then ..." would become "OP_DEF_FUNC 2 35 36"
    //assuming that 35 corresponds to "x" and 36 corresponds to "y" in the
    //constant table.
    OP_DEF_FUNC
};

/**
 * The compiled contents of a module. This contains none of the values
 * generated at runtime.
 */
typedef struct {
    //the constant table, constantsLength is the length of constants
    uint32_t constantsLength;
    const char** constants;

    //the bytecode, bytecodeLength is the length of bytecode
    uint32_t bytecodeLength;
    const unsigned char* bytecode;
} Module;

#endif /* BYTECODE_H_ */
