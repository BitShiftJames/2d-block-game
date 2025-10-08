#ifndef JAM_LIGHTING_H
#define JAM_LIGHTING_H

#include "jamTypes.h"
#include "jamTiles.h"
#include "jamMath.h"
#include "stdio.h"
#include <cstdio>

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

// Alpha is default to max.
u16 packR4G4B4AF(u16 r, u16 g, u16 b) {
  u16 Result = (jamClamp_u16(r, 0, 15) << 12) | 
               (jamClamp_u16(g, 0, 15) << 8) | 
               (jamClamp_u16(b, 0, 15) << 4) | 15; 

  return Result;
}

u16 packR4G4B4A4(u16 r, u16 g, u16 b, u16 a) {
  u16 Result = (jamClamp_u16(r, 0, 15) << 12) | 
               (jamClamp_u16(g, 0, 15) << 8) | 
               (jamClamp_u16(b, 0, 15) << 4) |
               (jamClamp_u16(a, 0, 15)); 

  return Result;
}

unpackedR4G4B4A4 unpackR4G4B4A4(u16 color) {
  unpackedR4G4B4A4 Result = {};
  
  Result.r = (color >> 12) & 0xF;
  Result.g = (color >> 8) & 0xF;
  Result.b = (color >> 4) & 0xF;
  Result.a = (color >> 0) & 0xF;

  return Result; 
}

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

      unpackedR4G4B4A4 color = unpackR4G4B4A4(currentTile.light);
      jamColor fullColor = {(u8)(((f32)color.r / 15.0f) * 255), (u8)(((f32)color.g / 15.0f) * 255), (u8)(((f32)color.b / 15.0f) * 255), 255};
      setLightValue(injectValues, LightTextureDim, 
                    fullColor, LightMapX, LightMapY);

    }
  }
}

void PropagateLighting(jamColor *prevValues, jamColor *nextValues, u32 LightTextureDim) {
  s32 LightFallOff = 15;
  f32 attenuation = 0.878f; // there is probably a bug in this still because turning the attenuation any lower brings in the band (ing).

  for (s32 LightMapY = 0; LightMapY < LightTextureDim; LightMapY++) {
    for (s32 LightMapX = 0; LightMapX < LightTextureDim; LightMapX++) {
      
      jamColor currentLight = prevValues[LightMapY * LightTextureDim + LightMapX];

      jamColor selfLight = nextValues[LightMapY * LightTextureDim + LightMapX];
      selfLight.r = AddClampColorChannel(selfLight.r, (u8)((f32)currentLight.r * attenuation), 0, 255);
      selfLight.g = AddClampColorChannel(selfLight.g, (u8)((f32)currentLight.g * attenuation), 0, 255);
      selfLight.b = AddClampColorChannel(selfLight.b, (u8)((f32)currentLight.b * attenuation), 0, 255);
      nextValues[LightMapY * LightTextureDim + LightMapX] = selfLight;

      for (s32 NeighborMapY = LightMapY - 1; NeighborMapY <= LightMapY + 1; NeighborMapY++) {
      for (s32 NeighborMapX = LightMapX - 1; NeighborMapX <= LightMapX + 1; NeighborMapX++) {
          if (NeighborMapX == LightMapX && NeighborMapY == LightMapY) { continue; } // We skip over this value in the loop because it's simpler to think that we already set the current value.

          if (NeighborMapX < LightTextureDim && NeighborMapY < LightTextureDim && NeighborMapX >= 0 && NeighborMapY >= 0) {
            jamColor NeighborLight = prevValues[NeighborMapY * LightTextureDim + NeighborMapX];

            if (NeighborLight.r < currentLight.r && currentLight.r != 0) {
              u8 B = (u8)(Maximum(0, (s32)currentLight.r - LightFallOff));
              nextValues[NeighborMapY * LightTextureDim + NeighborMapX].r = B;
            }

            if (NeighborLight.g < currentLight.g && currentLight.g != 0) {
              u8 B = (u8)(Maximum(0, (s32)currentLight.g - LightFallOff));
              nextValues[NeighborMapY * LightTextureDim + NeighborMapX].g = B;
            }

            if (NeighborLight.b < currentLight.b && currentLight.b != 0) {
              u8 B = (u8)(Maximum(0, (s32)currentLight.b - LightFallOff));
              nextValues[NeighborMapY * LightTextureDim + NeighborMapX].b = B;
            }

            nextValues[NeighborMapY * LightTextureDim + NeighborMapX].a = 255;
            
          } 

      }
      }

    }
  }

  fflush(stdout);
}

#endif
