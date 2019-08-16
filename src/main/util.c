#include <stdlib.h>
#include <string.h>

const char* newStr(const char* src) {
    int len = sizeof(char) * (strlen(src) + 1);
    char* dst = (char*) malloc(len);
    memcpy(dst, src, len);
    dst[len - 1] = 0;
    return dst;
}
