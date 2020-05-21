#include "main/bytecode.h"

BytecodeSrcLoc createBytecodeSrcLoc(uint32_t index, SrcLoc location) {
    BytecodeSrcLoc srcLoc;
    srcLoc.index = index;
    srcLoc.location = location;
    return srcLoc;
}
