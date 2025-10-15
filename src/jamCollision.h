#ifndef JAM_COLLISION_H
#define JAM_COLLISION_H

#include "jamTypes.h"
#include "jamMath.h"
// fuck windows
inline b32 PointInRect(jam_rect2 A, Vector2 Point) {
  b32 withinX = Point.x > A.x && Point.x < A.Max.x;
  b32 withinY = Point.y > A.y && Point.y < A.Max.y;
  
  b32 within = withinX && withinY;

  return within;
}

static inline b32 AABBcollisioncheck(jam_rect2 A, jam_rect2 B) {
  b32 left = A.Max.x < B.x;
  b32 right = A.x > B.Max.x;
  b32 bottom = A.Max.y < B.y;
  b32 top = A.y > B.Max.y;
  
  return !(left   ||
           right  ||
           bottom ||
           top);
}

static jam_rect2 rectangle_overlap(jam_rect2 A, jam_rect2 B) {
  jam_rect2 Result = {};
  
  Result.x = B.x - A.Max.x;
  Result.Max.x = B.Max.x - A.x;
  Result.y = B.y - A.Max.y;
  Result.Max.y = B.Max.y - A.y;
  
  return Result;
}

#endif
