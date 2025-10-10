#ifndef JAMTYPES_H
#define JAMTYPES_H

// TODO: Get rid of C standard library? looked through the header and can't read how to actually set these myself.
#include <stdint.h>

#define jamLIGHTDEBUB_ 0
#define jamMEMORYHOG_ 1

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u32 b32;

typedef int8_t s8;
typedef uint16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#endif
