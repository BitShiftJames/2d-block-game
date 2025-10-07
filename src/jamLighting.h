#ifndef JAM_LIGHTING_H
#define JAM_LIGHTING_H

#include "jamTypes.h"
#include "jamTiles.h"
#include "jamMath.h"

typedef struct jamColor {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} jamColor;

jamColor AddClampColor(jamColor A, jamColor B, u16 low, u16 high) {
  jamColor Result = {};

  Result.r = (u8)jamClamp_u16(((u16)A.r + (u16)B.r), low, high);
  Result.g = (u8)jamClamp_u16(((u16)A.g + (u16)B.g), low, high);
  Result.b = (u8)jamClamp_u16(((u16)A.b + (u16)A.b), low, high);

  Result.a = 255; // for debug texture to work correct, yes this is hacky.
  return Result;
}

u8 AddClampColorChannel(u8 A, u8 B, u8 low, u8 high) {
  u8 Result = 0;

  Result = (u8)jamClamp_u16(((u16)A + (u16)B), (u16)low, (u16)high);

  return Result;
}

void setLightValue(jamColor *imageValues, u32 LightTextureDim, jamColor A, u8 X, u8 Y) {
  // only setting the alpha because the debug light texture needs it.
  imageValues[Y * LightTextureDim + X] = jamColor{A.r, A.g, A.b, 255};
}

void InjectLighting(jamColor *injectValues, world global_world, jam_rect2 render_rectangle, u32 LightTextureDim) {
  for (u32 LightMapY = 0; LightMapY < LightTextureDim; LightMapY++) {
    for (u32 LightMapX = 0; LightMapX < LightTextureDim; LightMapX++) {
      u32 TileLightX = render_rectangle.Min.x + LightMapX;
      u32 TileLightY = render_rectangle.Min.y + LightMapY;

      if (TileLightX > global_world.Width || TileLightY > global_world.Height) { continue; }
      
      tile currentTile = getTile(global_world, TileLightX, TileLightY);
      setLightValue(injectValues, LightTextureDim, 
                    jamColor{currentTile.light, currentTile.light, currentTile.light, 255}, 
                    LightMapX, LightMapY);

    }
  }
}

void FakeInjectLighting(jamColor *injectValues, world global_world, jam_rect2 render_rectangle, f32 LightTextureDim) {
  setLightValue(injectValues, LightTextureDim, jamColor{0, 255, 0, 255}, 127, 127);
  setLightValue(injectValues, LightTextureDim, jamColor{255, 0, 0, 255}, 30, 127);
  setLightValue(injectValues, LightTextureDim, jamColor{0, 0, 255, 255}, 230, 127);
}

void PropagateLighting(jamColor *prevValues, jamColor *nextValues, u32 LightTextureDim) {
  f32 LightFallOff = 0.075f;
  for (u32 LightMapY = 0; LightMapY < LightTextureDim; LightMapY++) {
    for (u32 LightMapX = 0; LightMapX < LightTextureDim; LightMapX++) {
      
      jamColor currentLight = prevValues[LightMapY * LightTextureDim + LightMapX];

      jamColor selfLight = nextValues[LightMapY * LightTextureDim + LightMapX];
      selfLight.r = AddClampColorChannel(selfLight.r, (u8)((f32)currentLight.r * (1.0f - LightFallOff)), 0, 255);
      selfLight.g = AddClampColorChannel(selfLight.g, (u8)((f32)currentLight.g * (1.0f - LightFallOff)), 0, 255);
      selfLight.b = AddClampColorChannel(selfLight.b, (u8)((f32)currentLight.b * (1.0f - LightFallOff)), 0, 255);
      nextValues[LightMapY * LightTextureDim + LightMapX] = selfLight;

      for (u32 NeighborMapY = LightMapY - 1; NeighborMapY <= LightMapY + 1; NeighborMapY++) {
      for (u32 NeighborMapX = LightMapX - 1; NeighborMapX <= LightMapX + 1; NeighborMapX++) {
          if (NeighborMapX == LightMapX && NeighborMapY == LightMapY) { continue; } // We skip over this value in the loop because it's simpler to think that we already set the current value.

          if (NeighborMapX < LightTextureDim && NeighborMapY < LightTextureDim) {
            jamColor NeighborLight = prevValues[NeighborMapY * LightTextureDim + NeighborMapX];

            if (NeighborLight.r < currentLight.r) {
              u8 B = (u8)((currentLight.r - NeighborLight.r) * 0.250f);
              nextValues[NeighborMapY * LightTextureDim + NeighborMapX].r = B;
            }

            if (NeighborLight.g < currentLight.g) {
              u8 B = (u8)((currentLight.g - NeighborLight.g) * 0.250f);
              nextValues[NeighborMapY * LightTextureDim + NeighborMapX].g = B;
            }

            if (NeighborLight.b < currentLight.b) {
              u8 B = (u8)((currentLight.b - NeighborLight.b) * 0.250f);
              nextValues[NeighborMapY * LightTextureDim + NeighborMapX].b = B;
            }
          } // using the power of the u32 we can just assume that if it is greater than it's out of bounds.

      }
      }

    }
  }

}

void FinalLighting(jamColor *updateValues, jamColor *outputValues, u32 LightTextureDim) {
  for (u32 LightMapY = 0; LightMapY < LightTextureDim; LightMapY++) {
    for (u32 LightMapX = 0; LightMapX < LightTextureDim; LightMapX++) {
        outputValues[LightMapY * LightTextureDim + LightMapX] = updateValues[LightMapY * LightTextureDim + LightMapX];       
    }
  }
}

#endif
