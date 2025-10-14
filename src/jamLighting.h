#ifndef JAM_LIGHTING_H
#define JAM_LIGHTING_H

#include "jamEntities.h"
#include "jamTypes.h"
#include "jamTiles.h"
#include "jamMath.h"
#include "stdio.h"

typedef struct jamColor {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} jamColor;

typedef struct unpackedR4G4B4A4 {
  s32 r;
  s32 g;
  s32 b;
  s32 a;
} unpackedR4G4B4A4;

u8 u16Valuestou32Values[16] = {
  0,
  17,
  34,
  51,
  68,
  85,
  102,
  119,
  136,
  153,
  170,
  187,
  204,
  221, 
  238,
  255,
};
// Alpha is default to max.
static inline u16 packR4G4B4AF(u16 r, u16 g, u16 b) {
  u16 Result = (jamClamp_u16(r, 0, 15) << 12) | 
               (jamClamp_u16(g, 0, 15) << 8) | 
               (jamClamp_u16(b, 0, 15) << 4) | 15; 

  return Result;
}

static inline u16 packR4G4B4A4(u16 r, u16 g, u16 b, u16 a) {
  u16 Result = (jamClamp_u16(r, 0, 15) << 12) | 
               (jamClamp_u16(g, 0, 15) << 8) | 
               (jamClamp_u16(b, 0, 15) << 4) |
               (jamClamp_u16(a, 0, 15)); 

  return Result;
}

static inline unpackedR4G4B4A4 unpackR4G4B4A4(u16 color) {
  unpackedR4G4B4A4 Result = {};
  
  Result.r = (color >> 12) & 0xF;
  Result.g = (color >> 8) & 0xF;
  Result.b = (color >> 4) & 0xF;
  Result.a = (color >> 0) & 0xF;

  return Result; 
}

static inline jamColor AddClampColor(jamColor A, jamColor B, u16 low, u16 high) {
  jamColor Result = {};

  Result.r = (u8)jamClamp_u16(((u16)A.r + (u16)B.r), low, high);
  Result.g = (u8)jamClamp_u16(((u16)A.g + (u16)B.g), low, high);
  Result.b = (u8)jamClamp_u16(((u16)A.b + (u16)A.b), low, high);

  Result.a = 255; // for debug texture to work correct, yes this is hacky.
  return Result;
}

static inline u8 AddClampColorChannel(u8 A, u8 B, u8 low, u8 high) {
  u8 Result = 0;

  Result = (u8)jamClamp_u16(((u16)A + (u16)B), (u16)low, (u16)high);

  return Result;
}

static inline void setLightValue(jamColor *imageValues, u32 LightTextureDim, jamColor A, u8 X, u8 Y) {
  // only setting the alpha because the debug light texture needs it.
  imageValues[Y * LightTextureDim + X] = jamColor{A.r, A.g, A.b, 255};
}

static void InjectLighting(jamColor *injectValues, world global_world, total_entities global_entities, jam_rect2 render_rectangle, u32 LightTextureDim) {
  TIMED_BLOCK();
  for (u32 LightMapY = 0; LightMapY < LightTextureDim; LightMapY++) {
    for (u32 LightMapX = 0; LightMapX < LightTextureDim; LightMapX++) {
      TIMED_BLOCK();
      u32 TileLightX = (u32)render_rectangle.Min.x + LightMapX;
      u32 TileLightY = (u32)render_rectangle.Min.y + LightMapY;

      if ((TileLightX >= global_world.Width || TileLightY >= global_world.Height)) { continue; }
      
      tile currentTile = global_world.map[TileLightY * global_world.Width + TileLightX];
      
      u8 colorR = (currentTile.light >> 12) & 0xF;
      u8 colorB = (currentTile.light >> 8) & 0xF;
      u8 colorG = (currentTile.light >> 4) & 0xF;
      u8 colorA = (currentTile.light >> 0) & 0xF;

      injectValues[LightMapY * LightTextureDim + LightMapX] = jamColor{u16Valuestou32Values[colorR], u16Valuestou32Values[colorG], u16Valuestou32Values[colorB], u16Valuestou32Values[colorA]};

    }
  }
}

static void PropagateLighting(jamColor *prevValues, jamColor *nextValues, u32 LightTextureDim) {
  TIMED_BLOCK();
  s32 LightFallOff = 15;

  for (s32 LightMapY = 0; LightMapY < LightTextureDim; LightMapY++) {
    for (s32 LightMapX = 0; LightMapX < LightTextureDim; LightMapX++) {
      TIMED_BLOCK();
      u32 middleIndex = LightMapY * LightTextureDim + LightMapX;
      jamColor currentLight = prevValues[middleIndex];

      if (nextValues[middleIndex].r == 0 && nextValues[middleIndex].g == 0 && nextValues[middleIndex].b == 0) {
        nextValues[middleIndex] = prevValues[middleIndex];
      } else {
        nextValues[middleIndex] = nextValues[middleIndex];
      }

      for (s32 NeighborMapY = LightMapY - 1; NeighborMapY <= LightMapY + 1; NeighborMapY++) {
      for (s32 NeighborMapX = LightMapX - 1; NeighborMapX <= LightMapX + 1; NeighborMapX++) {
          if (NeighborMapX == LightMapX && NeighborMapY == LightMapY) { continue; } // We skip over this value in the loop because it's simpler to think that we already set the current value.
          u32 nearIndex = NeighborMapY * LightTextureDim + NeighborMapX;
          if (NeighborMapX < LightTextureDim && NeighborMapY < LightTextureDim && NeighborMapX >= 0 && NeighborMapY >= 0) {
            jamColor NeighborLight = prevValues[nearIndex];
            jamColor& nextNeighborLight = nextValues[nearIndex];

            if (NeighborLight.r < currentLight.r) {
              u8 A = (u8)(Maximum(0, (s32)currentLight.r - LightFallOff));
              nextValues[nearIndex].r = A;
            }

            if (NeighborLight.g < currentLight.g) {
              u8 B = (u8)(Maximum(0, (s32)currentLight.g - LightFallOff));
              nextValues[nearIndex].g = B;
            }

            if (NeighborLight.b < currentLight.b) {
              u8 C = (u8)(Maximum(0, (s32)currentLight.b - LightFallOff));
              nextValues[nearIndex].b = C;
            }

            nextValues[nearIndex].a = 255;
            
          } 

      }
      }

    }
  }

}

#endif
