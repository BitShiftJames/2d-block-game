#ifndef JAM_ENTITIES_H
#define JAM_ENTITIES_H

#include "jamTypes.h"
#include "jamMath.h"
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

// this function is probably not needed I am just making it a function so I can debug
// the render stack.
void player_acceleration(entity *Entity, f32 inputStrength) {
    // acceleration clear to 0 moved at the end of each frame. Within the entity loop.
    Entity->acceleration = {0, 0};
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
    Entity->acceleration += -2.0f * Entity->velocity;
}

// it might be worth to do two entity loops rendering and logic
// with a max entity of 256 two loops of 256 isn't really that bad.
// although with spatial hashing that will probably be even less.
void entity_loop(total_entities *global_entities) {
  for (u8 entity_index = 0; entity_index < global_entities->entity_count; entity_index++) {
    entity currentEntity = global_entities->entities[entity_index];
    DrawRectangleV(Vector2{currentEntity.pos.x, currentEntity.pos.y}, 
                   Vector2{currentEntity.dim.x, currentEntity.dim.y}, WHITE);
  }
}

#endif
