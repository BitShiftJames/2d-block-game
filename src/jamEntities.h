#ifndef JAM_ENTITIES_H
#define JAM_ENTITIES_H

#include "jamTypes.h"
#include "jamMath.h"
#include "jamTiles.h"
#include "raylib.h"

enum entity_states {
  IGNORE,
  IDLE,
  WONDER,
  CHASE,
  ATTACK,
};

struct entity {
  u8 state;
  v2 pos;
  v2 velocity;
  v2 acceleration;
  v2 dim;
  b32 debug_render;
};

struct total_entities {
  u8 entity_count;
  entity entities[256];
};

b32 AABBcollisioncheck(jam_rect2 A, jam_rect2 B) {
  b32 left = A.x + A.Max.x < B.x;
  b32 right = A.x > B.x + B.Max.x;
  b32 bottom = A.y + A.Max.y < B.y;
  b32 top = A.y > B.Max.y + B.y;
  
  return !(left   ||
           right  ||
           bottom ||
           top);
}

jam_rect2 rectangle_overlap(jam_rect2 A, jam_rect2 B) {
  jam_rect2 Result = {};
  
  Result.x = B.x - A.Max.x;
  Result.Max.x = B.Max.x - A.x;
  Result.y = B.y - A.Max.y;
  Result.Max.y = B.Max.y - A.y;
  
  return Result;
}

u8 add_entity(total_entities *global_entities, v2 dim, v2 pos, entity_states State, b32 debug_state) {
  u8 entity_id;
  if (global_entities->entity_count < ArrayCount(global_entities->entities) - 1) {
    global_entities->entities[global_entities->entity_count].dim = dim;
    global_entities->entities[global_entities->entity_count].pos = pos;
    global_entities->entities[global_entities->entity_count].state = State;
    global_entities->entities[global_entities->entity_count].debug_render = debug_state;

    entity_id = global_entities->entity_count++;
  }

  return entity_id;
}

v2 generate_delta_movement(entity Entity, f32 deltaTime) {
  v2 Result = {};

  Result = (.5 * Entity.acceleration * (deltaTime * deltaTime)) + (Entity.velocity * deltaTime);

  return Result;
}

void collision_resolution_for_move(entity *Entity, world global_world, v2 delta_movement, f32 deltaTime) {

    v2 new_entity_position = Entity->pos + delta_movement;

    v2 Min = {Minimum(Entity->pos.x, new_entity_position.x), 
              Minimum(Entity->pos.y, new_entity_position.y)};

    v2 Max = {Maximum(Entity->pos.x + Entity->dim.x, new_entity_position.x + Entity->dim.x), 
              Maximum(Entity->pos.y + Entity->dim.y, new_entity_position.y + Entity->dim.y)};

    jam_rect2 player_collision_rect = JamRectMinMax(Min, Max);

    f32 tMin = 1.0f;
    v2 normal = {};

    u32 MinTileX = (floor_f32(Min.x)) / global_world.TileSize;
    u32 MinTileY = (floor_f32(Min.y)) / global_world.TileSize;
    u32 MaxTileX = (floor_f32(Max.x)) / global_world.TileSize;
    u32 MaxTileY = (floor_f32(Max.y)) / global_world.TileSize;
    jam_rect2 TileRect = {};
    for (u32 TileY = MinTileY; TileY <= MaxTileY; TileY++) {
      for (u32 TileX = MinTileX; TileX <= MaxTileX; TileX++) {
        // general accessor function incoming.
        tile CurrentTile = global_world.map[TileY * global_world.Width + TileX];
        // okay new theory I need to construct an actual rectangle from all the connecting tiles
        // this assumes a perfect AABB tile collision rect which is 90% of cases but if I want entities to have tile aligned dimensions
        // it would be best to do a field of rectangles to do AABB collision on. That way I can have even more odd collision behaviors
        // 0. look at all tiles overlaping the player
        // 1. Construct a single rectangle out of the tiles (this will make it so entities can not have tile aligned dimensions)
        // 2. use constructed rectangle to resolve collision
        if (CurrentTile.type != 0) {
          jam_rect2 current_tile_rectangle = JamRectMinDim(v2{(f32)TileX * global_world.TileSize, (f32)TileY * global_world.TileSize}, global_world.TileSize);

          if (TileRect.Min == TileRect.Max) {
            TileRect = current_tile_rectangle;
          } else {
            v2 Min = v2{Minimum(TileRect.Min.x, current_tile_rectangle.Min.x), Minimum(TileRect.Min.y, current_tile_rectangle.Min.y)}; 
            v2 Max = v2{Maximum(TileRect.Max.x, current_tile_rectangle.Max.x), Maximum(TileRect.Max.y, current_tile_rectangle.Max.y)};
            
            TileRect = JamRectMinMax(Min, Max);
          }

        }

      }
    }

    if (AABBcollisioncheck(player_collision_rect, TileRect)) {
      jam_rect2 overlap = rectangle_overlap(player_collision_rect, TileRect);
      v2 Minimum_overlap = {};
      
      if (overlap.Max.x > fabsf(overlap.x)) {
        Minimum_overlap.x = overlap.x;
      } else {
        Minimum_overlap.x = overlap.Max.x;
      }

      if (overlap.Max.y > fabsf(overlap.y)) {
        Minimum_overlap.y = overlap.y;
      } else {
        Minimum_overlap.y = overlap.Max.y;
      }

      if ((fabsf(Minimum_overlap.x) > fabsf(Minimum_overlap.y))) {
        if (delta_movement.y != 0.0f) {
          if (Minimum_overlap.y < 0.0f) {
            normal.y = 1.0f;
          } else {
            normal.y = -1.0f;
          }
          tMin = Minimum_overlap.y / delta_movement.y;
        }
      } else {
        if (delta_movement.x != 0.0f) {
          if (Minimum_overlap.x < 0.0f) {
            normal.x = -1.0f;
          } else {
            normal.x = 1.0f;
          }
          tMin = Minimum_overlap.x / delta_movement.x;
        }
      }
    }

    Entity->pos += delta_movement * (tMin);
    
    f32 normalLength = LengthSq(normal);
    if (normalLength > 0.0f) {
      Entity->velocity = Entity->velocity - 1*Inner(Entity->velocity, normal) * normal;
      Entity->acceleration = Entity->acceleration - Inner(Entity->acceleration, normal) * normal;
    } else {
    }

    Entity->velocity += Entity->acceleration * deltaTime;
}
// it might be worth to do two entity loops rendering and logic
// with a max entity of 256 two loops of 256 isn't really that bad.
// although with spatial hashing that will probably be even less.
void entity_loop(total_entities *global_entities, world global_world, f32 deltaTime) {
  for (u8 entity_index = 0; entity_index < global_entities->entity_count; entity_index++) {
    entity currentEntity = global_entities->entities[entity_index];

    v2 entity_movement_delta = generate_delta_movement(global_entities->entities[entity_index], 
                                                       deltaTime);

    collision_resolution_for_move(&global_entities->entities[entity_index], global_world, 
                                  entity_movement_delta, deltaTime);

    DrawRectangleV(Vector2{currentEntity.pos.x, currentEntity.pos.y}, 
                   Vector2{currentEntity.dim.x, currentEntity.dim.y}, WHITE);
  }
}

#endif
