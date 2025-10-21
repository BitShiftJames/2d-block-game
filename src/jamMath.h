#ifndef JAM_MATH_H
#define JAM_MATH_H

#include "jamTypes.h"
#include "raylib.h"

// TODO: Get rid of C standard library.
#include <math.h>


// yes I know I could have made a macro by now but I just don't want to deal with the language specifics for like an hour or two.

f32 jamClamp_f32(f32 x, f32 low, f32 high) {
  return x > high ? high : (x < low ? low : x);
}

s8 jamClamp_s8(s8 x, s8 low, s8 high) {
  return x > high ? high : (x < low ? low : x);
}

s16 jamClamp_s16(s16 x, s16 low, s16 high) {
  return x > high ? high : (x < low ? low : x);
}

s32 jamClamp_s32(s32 x, s32 low, s32 high) {
  return x > high ? high : (x < low ? low : x);
}

s64 jamClamp_s64(s64 x, s64 low, s64 high) {
  return x > high ? high : (x < low ? low : x);
}

u8 jamClamp_u8(u8 x, u8 low, u8 high) {
  return x > high ? high : (x < low ? low : x);
}

u16 jamClamp_u16(u16 x, u16 low, u16 high) {
  return x > high ? high : (x < low ? low : x);
}

#define JAMRAND_MAX 0xFFFFFFFF

#define Rand() ((u32)GetRandomValue(0, JAMRAND_MAX))

// when a world is saved we can get a file Mod time 
// GetFileModTime(const char *fileName);
// and use that to set the initial value. For continued precieved randomness we could do something.
// like periodic take the file mod time and re seed the random function in raylib.
//SetRandomSeed(unsigned int seed);

static inline Vector2 JamToRayVec2(v2 A) {
  Vector2 Result = {};

  Result.x = A.x;
  Result.y = A.y;

  return Result;
}

static inline Vector3 JamToRayVec3(v3 A) {
  Vector3 Result = {};

  Result.x = A.x;
  Result.y = A.y;
  Result.z = A.z;
  
  return Result;
}

static inline Vector4 JamToRayVec4(v4 A) {
  Vector4 Result = {};

  Result.x = A.x;
  Result.y = A.y;
  Result.z = A.z;
  Result.w = A.w;

  return Result;
}

static inline Rectangle JamToRayRect(jam_rect2 A) {
  Rectangle Result = {};

  Result.x = A.x;
  Result.y = A.y;
  Result.width = A.Max.x - A.x;
  Result.height = A.Max.y - A.y;

  return Result;
}


static inline jam_rect2 JamRectMinDim(Rectangle A) {
  jam_rect2 Result = {};

  Result.x = A.x;
  Result.y = A.y;
  Result.Max.x = A.x + A.width;
  Result.Max.y = A.y + A.height;

  return Result;
}

static inline f32 floor_f32(f32 value) {
  f32 result = floorf(value);

  return result;
}

static inline f32 SquareRoot(f32 value) {
  f32 Result = sqrtf(value);

  return Result;
}

static inline f32 Inner(v2 A, v2 B) {
  f32 Result = A.x * B.x + A.y * B.y;

  return Result;
}

static inline f32 LengthSq(v2 A) {
  f32 Result = Inner(A, A);
  
  return Result;
}

static inline f32 Length(v2 A) {
  f32 Result = SquareRoot(LengthSq(A));

  return Result;
}

#endif
