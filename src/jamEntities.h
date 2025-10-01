#ifndef JAM_ENTITIES_H
#define JAM_ENTITIES_H

#include "jamTypes.h"
#include "jamMath.h"
#include "jamTiles.h"
#include "raylib.h"
#include "math.h"
#include <cstdlib>

enum entity_states {
  IGNORE,
  IDLE,
  WONDER,
  CHASE,
  ATTACK,
};

struct entity {
  u32 state;
  v2 pos;
  v2 velocity;
  v2 acceleration; // this could be sparse? depends if I need the space.
  v2 dim;
  i32 stateTime; // Max amount of seconds that an entity can be in a state.
  b32 debug_render; // this is a boolean later on it will probably be collapsed into flags.
};

struct total_entities {
  u8 entity_count;
  entity entities[256];
};

b32 AABBcollisioncheck(jam_rect2 A, jam_rect2 B) {
  b32 left = A.Max.x < B.x;
  b32 right = A.x > B.Max.x;
  b32 bottom = A.Max.y < B.y;
  b32 top = A.y > B.Max.y;
  
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

jam_rect2 collision_rect_construction(jam_rect2 A, world global_world) {
    u32 MinTileX = (floor_f32(A.Min.x)) / global_world.TileSize;
    u32 MinTileY = (floor_f32(A.Min.y)) / global_world.TileSize;
    u32 MaxTileX = (floor_f32(A.Max.x)) / global_world.TileSize;
    u32 MaxTileY = (floor_f32(A.Max.y)) / global_world.TileSize;

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

  return TileRect;

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

v2 generate_delta_movement(entity *Entity, world global_world, f32 deltaTime) {
  v2 Result = {};
  // raycast or some check to see if there is something below the player.
  f32 padding = 1;
  f32 drag_coefficent = 2.0f;
  jam_rect2 ground_box = JamRectMinDim(v2{Entity->pos.x + padding, Entity->pos.y + (Entity->dim.y - padding)}, v2{Entity->dim.x - (padding * 2), 4});
  jam_rect2 tile_box = collision_rect_construction(ground_box, global_world);
  if (!AABBcollisioncheck(ground_box, tile_box)) {
    Entity->acceleration.y += global_world.gravity_constant;
  }
  Entity->acceleration += -drag_coefficent * Entity->velocity;
  Result = (.5 * Entity->acceleration * (deltaTime * deltaTime)) + (Entity->velocity * deltaTime);

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
    jam_rect2 TileRect = collision_rect_construction(player_collision_rect, global_world);

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

void entity_wonder(entity *Entity) {

    if (Entity->stateTime <= 0) {
      Entity->stateTime = 10;
      Entity->state = IDLE;
    }

    f32 speed = 50;
    if (Entity->velocity.x > 0) {
      Entity->acceleration = v2{1.0f * speed, 0};
    } else {
      Entity->acceleration = v2{-1.0f * speed, 0};
    }


}

void entity_idle(entity *Entity) {
    if (Entity->stateTime <= 0) {
      Entity->stateTime = 5;
      Entity->state = WONDER;
      f32 direction = (abs(rand()) > (RAND_MAX / 2) ? -1.0 : 1.0);
      f32 speed = 200;
      Entity->acceleration = v2{direction * speed, 0};
    } 

}

void entity_ignore(entity *Entity, f32 inputStrength) {
    if (IsKeyDown(KEY_W)) {
      Entity->acceleration.y--;
    }
    if (IsKeyDown(KEY_A)) {
      Entity->acceleration.x--;
    }
    if (IsKeyDown(KEY_S)) {
      Entity->acceleration.y++;
    }
    if (IsKeyDown(KEY_D)) {
      Entity->acceleration.x++;
    }

    f32 ddPosLength = LengthSq(Entity->acceleration);
    if (ddPosLength > 1.0f) {
      Entity->acceleration *= 1.0f / SquareRoot(ddPosLength);
    }
    Entity->acceleration *= inputStrength;
}

void entity_loop(total_entities *global_entities, world global_world, f32 deltaTime, f32 OneSecond) {
  for (u8 entity_index = 0; entity_index < global_entities->entity_count; entity_index++) {
    entity currentEntity = global_entities->entities[entity_index];
    f32 inputStrength = 250.0f;
    global_entities->entities[entity_index].acceleration = {0, 0};

    if (currentEntity.state && (OneSecond >= 1.0f)) {
      global_entities->entities[entity_index].stateTime -= 1;
    }

    switch (currentEntity.state) {
        case IGNORE: {
          entity_ignore(&global_entities->entities[entity_index], inputStrength);
        } break;
        case IDLE: {
          entity_idle(&global_entities->entities[entity_index]);
        } break;
        case WONDER: {
          entity_wonder(&global_entities->entities[entity_index]);
        } break;
        case CHASE: {
        } break;
        case ATTACK: {
        } break;
        default: {
          // this is an error.
        }
    }
    v2 entity_movement_delta = generate_delta_movement(&global_entities->entities[entity_index], global_world,
                                                       deltaTime);

    collision_resolution_for_move(&global_entities->entities[entity_index], global_world, 
                                  entity_movement_delta, deltaTime);
    
    switch (currentEntity.state) {
      case IGNORE: {
        DrawRectangleV(Vector2{currentEntity.pos.x, currentEntity.pos.y}, 
                       Vector2{currentEntity.dim.x, currentEntity.dim.y}, WHITE);
      } break;
      case IDLE: {
        DrawRectangleV(Vector2{currentEntity.pos.x, currentEntity.pos.y}, 
                       Vector2{currentEntity.dim.x, currentEntity.dim.y}, GREEN);
      } break;
      case WONDER: {
        DrawRectangleV(Vector2{currentEntity.pos.x, currentEntity.pos.y}, 
                       Vector2{currentEntity.dim.x, currentEntity.dim.y}, YELLOW);
      } break;
      case CHASE: {
      } break;
      case ATTACK: {
      } break;
    }
  }
}

#endif
