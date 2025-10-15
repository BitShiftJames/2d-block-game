#ifndef JAMTYPES_H
#define JAMTYPES_H

// TODO: Get rid of C standard library? looked through the header and can't read how to actually set these myself.
#include <stdint.h>

#define Minimum(a, b) ((a) < (b) ? (a) : (b))
#define Maximum(a, b) ((a) > (b) ? (a) : (b))
#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

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
struct v2 {
  union {
    struct {
      f32 x;
      f32 y;
    };
    struct {
      f32 width;
      f32 height;
    };
    struct {
      f32 u;
      f32 v;
    };
    f32 E[2];
  };
};

struct jam_rect2 {
  union {
    struct {
      v2 Min;
      v2 Max;
    };
    struct {
      f32 x;
      f32 y;
      f32 z;
      f32 w;
    };
    f32 E[4];
  };
};

bool operator==(v2 A, v2 B) {
  return A.x == B.x &&
         A.y == B.y;
}

bool operator!=(v2 A, v2 B) {
  return A.x != B.x &&
         A.y != B.y;
}

v2 operator+(v2 A, v2 B) {
  v2 Result = {};

  Result.x = A.x + B.x;
  Result.y = A.y + B.y;

  return Result;
}

v2 operator-(v2 A, v2 B) {
  v2 Result = {};

  Result.x = A.x - B.x;
  Result.y = A.y - B.y;

  return Result;
}

v2 operator-(v2 A, f32 Scaler) {
  v2 Result = {};

  Result.x = A.x - Scaler;
  Result.y = A.y - Scaler;

  return Result;
}

v2 operator*(v2 A, v2 B) {
  v2 Result = {};

  Result.x = A.x * B.x;
  Result.y = A.y * B.y;

  return Result;
}

v2& operator*=(v2& A, f32 Scalar) {
  A.x *= Scalar;
  A.y *= Scalar;

  return A;
}

v2& operator*=(v2& A, v2 B) {
  A.x *= B.x;
  A.y *= B.y;

  return A;
}

v2& operator+=(v2& a, v2 b) {
  a.x += b.x;
  a.y += b.y;

  return a;
}

v2& operator+=(v2& a, f32 b) {
  a.x += b;
  a.y += b;

  return a;
}

v2 operator*(v2 A, f32 Scalar) {
  v2 Result = {};

  Result.x = A.x * Scalar;
  Result.y = A.y * Scalar;

  return Result;
}

v2 operator/(v2 A, f32 Scalar) {
  v2 Result = {};

  Result.x = A.x / Scalar;
  Result.y = A.y / Scalar;

  return Result;
}

v2 operator*(f32 Scalar, v2 A) {
  v2 Result = {};

  Result.x = A.x * Scalar;
  Result.y = A.y * Scalar;

  return Result;
}

struct v3 {
  union {
    struct {
      f32 x;
      f32 y;
      f32 z;
    };
    struct {
      f32 r;
      f32 g;
      f32 b;
    };
    f32 E[3];
  };
};

struct v4 {
  union {
    struct {
      f32 x;
      f32 y;
      f32 z;
      f32 w;
    };
    struct {
      f32 r;
      f32 g;
      f32 b;
      f32 a;
    };
    f32 E[4];
  };
};

#endif
