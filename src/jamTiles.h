#ifndef JAM_TILES_H
#define JAM_TILES_H

#include "jamTypes.h"

struct tile {
  u32 type;
  // shader information
  u8 light;
};

struct world {
  // maximum world size is 65,535
  u16 Width;
  u16 Height;

  f32 gravity_constant;

  u16 TileSize;
  tile *map;
};

tile getTile(world global_world, u32 TileX, u32 TileY) {
  tile Result = {};
  if (TileX < global_world.Width && TileY < global_world.Height) {
    Result = global_world.map[TileY * global_world.Width + TileX];
  }
  return Result;
}

#endif
