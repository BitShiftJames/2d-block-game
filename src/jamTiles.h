#ifndef JAM_TILES_H
#define JAM_TILES_H

#include "jamTypes.h"

struct tile {
  u32 type;
  // shader information
  u32 light;
};

struct world {
  // maximum world size is 65,535
  u16 Width;
  u16 Height;

  f32 gravity_constant;

  u16 TileSize;
  tile *map;
};

#endif
