#ifndef JAM_INTRINSICS_H
#define JAM_INTRINSICS_H

#include "jamTypes.h"

#include <intrin.h>
#include <xmmintrin.h>
// Windows fucking everything up since 1981.
// Also why can't I just tell the compiler or C that I don't want anything from windows outside of this.

inline u32 AtomicCompareExchangeU32(volatile u32 *Value, u32 New, u32 Expected) {
    u32 Result = _InterlockedCompareExchange((long *)Value, New, Expected);

    return Result;
}

inline u64 AtomicExchangeU64(volatile u64 *Value, u64 New) {
    u64 Result = _InterlockedExchange64((__int64 *)Value, New);

    return Result;
}

inline u64 AtomicAddU64(volatile u64 *Value, u64 Add) {
    u64 Result = _InterlockedExchangeAdd64((__int64 *)Value, Add);

    return Result;
}
#endif
