#ifndef JAM_ENTITIES_H
#define JAM_ENTITIES_H

#include "jamTypes.h"
#include "jamMath.h"
#include "jamTiles.h"
#include "jamCollision.h"
#include "raylib.h"

// 
// 22:26
//

#include "stdio.h"

#define MAX_ENTITIES 256

enum entity_states {
  INVALID,
  IGNORE,
  IDLE,
  WONDER,
  CHASE,
  ATTACK,
};

struct healthComponent {
  u32 EntityID;
  s32 Health;
  u32 EffectFlags;
};

struct fallComponent {
  u32 EntityID;

  b32 groundedLastFrame;
  f32 lastKnownGroundPosition;

};

u32 hash(u32 ID, u32 Table_size) {
  // TODO: Test this hash function to make sure it works over a large amount of IDs.
  u32 a = 1103515245;
  u32 c = 12345;
  u32 m = 1u << 31; 

  u32 Result = (a * ID + c) % m;

  Result = Result % Table_size;

  return Result;
}


struct entity {
  // TODO[ECS]: Components for sparseness.
  u32 EntityID;

  u32 state;
  v2 pos;
  v2 velocity;
  v2 acceleration; 
  v2 dim;
  s32 stateTime;

};

bool AddFallComponent(fallComponent *fallComponents, u32 ID) {
  bool Result = false;

  u32 hash_index = hash(ID, MAX_ENTITIES);
  for (s32 i=0; i < MAX_ENTITIES; i++) {
    s32 trying = (i + hash_index) % MAX_ENTITIES;
    if (fallComponents[trying].EntityID) {
      // Skip to next.
      continue;
    } else {
      fallComponents[trying].EntityID = ID;

      return true;
    }
  }

  return false;
}

// TODO: is the delete needed for a fall component. Probably if I do NPC's effected by fall damage.
bool DeleteFallComponent(fallComponent *fallComponents, u32 ID) {
  u32 hash_index = hash(ID, MAX_ENTITIES);
  for (s32 i = 0; i < MAX_ENTITIES; i++) {
    s32 trying = (i + hash_index) % MAX_ENTITIES;

    if (ID == fallComponents[trying].EntityID) {
      fallComponents[trying].EntityID = 0;
      fallComponents[trying].lastKnownGroundPosition = 0;
      return true;
    }
  }

  return false;
}

// TODO[ECS]: Should this be wrapped with a peek call or is this fine.
fallComponent *fallLookUp(fallComponent *fallComponents, u32 ID) {

  fallComponent *Result = {};

  u32 hash_index = hash(ID, MAX_ENTITIES);
  for (s32 i = 0; i < MAX_ENTITIES; i++) {
    s32 trying = (i + hash_index) % MAX_ENTITIES;

    if (ID == fallComponents[trying].EntityID) {
      Result = &fallComponents[trying];
      return Result;
    }

  }

  return Result;
}

// TODO[ECS]: Bounded peak or something like that. Limit the amount of iterations
bool fallPeak(fallComponent *fallComponents, u32 ID) {
  u32 hash_index = hash(ID, MAX_ENTITIES);
  for (s32 i = 0; i < MAX_ENTITIES; i++) {
    s32 trying = (i + hash_index) % MAX_ENTITIES;

    if (ID == fallComponents[trying].EntityID) {
      return true;
    }

  }

  return false;
}

bool AddHealthComponent(healthComponent *HealthComponents, u32 ID, u32 Health) {
  bool Result = false;

  u32 hash_index = hash(ID, MAX_ENTITIES);
  for (s32 i=0; i < MAX_ENTITIES; i++) {
    s32 trying = (i + hash_index) % MAX_ENTITIES;
    if (HealthComponents[trying].EntityID) {
      // Skip to next.
      continue;
    } else {
      HealthComponents[trying].EntityID = ID;
      HealthComponents[trying].Health = Health;
      return true;
    }
  }

  return false;
}

bool DeleteHealthComponent(healthComponent *HealthComponents, u32 ID) {
  u32 hash_index = hash(ID, MAX_ENTITIES);
  for (s32 i = 0; i < MAX_ENTITIES; i++) {
    s32 trying = (i + hash_index) % MAX_ENTITIES;

    if (ID == HealthComponents[trying].EntityID) {
      HealthComponents[trying].EntityID = 0;
      HealthComponents[trying].Health = 0;
      return true;
    }
  }

  return false;
}

bool HealthLookUp(healthComponent *HealthComponents, healthComponent *storeComponent, u32 ID) {
  u32 hash_index = hash(ID, MAX_ENTITIES);
  for (s32 i = 0; i < MAX_ENTITIES; i++) {
    s32 trying = (i + hash_index) % MAX_ENTITIES;

    if (ID == HealthComponents[trying].EntityID) {
      *storeComponent = HealthComponents[trying];
      return true;
    }

  }

  return false;
}


struct total_entities {
  u32 entity_count;
  u32 entity_id = 1;

  healthComponent HealthComponents[MAX_ENTITIES];
  // Using an entire list for fall components is kind of stupid.
  // Counter point. Dynamic memory allocation for certain list are incoming.
  fallComponent fallComponents[MAX_ENTITIES];
  entity entities[MAX_ENTITIES];
};

static u32 add_entity(total_entities *global_entities, v2 dim, v2 pos, entity_states State) {
  u32 entity_count;

  if (global_entities->entity_count < ArrayCount(global_entities->entities) - 1) {
    global_entities->entities[global_entities->entity_count].dim = dim;
    global_entities->entities[global_entities->entity_count].pos = pos;
    global_entities->entities[global_entities->entity_count].state = State;
    
    // TODO[ECS]: Start hashing and storing position in a hashtable because currently the entity ID is very fragile.
    // and also not up to spec with what the hash tables expect which is that a 0 entity_id is invalid.
    entity_count = global_entities->entity_count;
    global_entities->entities[global_entities->entity_count++].EntityID = global_entities->entity_id;
    
    printf("Entity ID: %u has a hash index of %u\n", global_entities->entity_id, hash(global_entities->entity_id, MAX_ENTITIES));
    global_entities->entity_id++;
  }

  return entity_count;
}

static jam_rect2 collision_rect_construction(jam_rect2 A, world global_world) {
    u32 MinTileX = (floor_f32(A.Min.x)) / global_world.TileSize;
    u32 MinTileY = (floor_f32(A.Min.y)) / global_world.TileSize;
    u32 MaxTileX = (floor_f32(A.Max.x)) / global_world.TileSize;
    u32 MaxTileY = (floor_f32(A.Max.y)) / global_world.TileSize;

    jam_rect2 TileRect = {};
    for (u32 TileY = MinTileY; TileY <= MaxTileY; TileY++) {
      for (u32 TileX = MinTileX; TileX <= MaxTileX; TileX++) {
        tile CurrentTile = getTile(global_world, TileX, TileY);
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

// TODO[ECS]: Do something smarter here instead of having to pass global_entities in everywhere.
static v2 generate_delta_movement(total_entities *global_entities, entity *Entity, world global_world, f32 deltaTime) {

  TIMED_BLOCK();
  v2 Result = {};
  
  f32 padding = 1;
  f32 drag_coefficent = 2.0f;
  jam_rect2 ground_box = JamRectMinDim(v2{Entity->pos.x + padding, Entity->pos.y + (Entity->dim.y - padding)}, v2{Entity->dim.x - (padding * 2), 1.2});
  jam_rect2 tile_box = collision_rect_construction(ground_box, global_world);

  fallComponent *storeComponent = {};
  if (fallPeak(global_entities->fallComponents, Entity->EntityID)) {
    storeComponent = fallLookUp(global_entities->fallComponents, Entity->EntityID);
  }

  if (!AABBcollisioncheck(ground_box, tile_box)) {
    Entity->acceleration.y += global_world.gravity_constant;

      // Null ptr badness.
      if (storeComponent) {
        if (storeComponent->groundedLastFrame) {
          storeComponent->groundedLastFrame = false;
          storeComponent->lastKnownGroundPosition = Entity->pos.y;
        }

      }

  } else {
    // Null ptr badness.
    if (storeComponent) {
    storeComponent->groundedLastFrame = true;
    }
  }
  
  Entity->acceleration += -drag_coefficent * Entity->velocity;
  Result = (.5 * Entity->acceleration * (deltaTime * deltaTime)) + (Entity->velocity * deltaTime);

  return Result;
}

static void collision_resolution_for_move(entity *Entity, world global_world, v2 delta_movement, f32 deltaTime) {
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
      // TODO[ECS]: Fix this.
      Entity->velocity = Entity->velocity - 1*Inner(Entity->velocity, normal) * normal;
      Entity->acceleration = Entity->acceleration - Inner(Entity->acceleration, normal) * normal;
    } else {
    }

    Entity->velocity += Entity->acceleration * deltaTime;
}

static void entity_wonder(entity *Entity) {
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

static void entity_idle(entity *Entity) {
    if (Entity->stateTime <= 0) {
      Entity->stateTime = 5;
      Entity->state = WONDER;
      f32 direction = (Rand() > (JAMRAND_MAX / 2)) ? -1.0f : 1.0f;
      f32 speed = 200;
      Entity->acceleration = v2{direction * speed, 0};
    } 
}

static void entity_ignore(entity *Entity, f32 inputStrength) {
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

//   Entity movement looks like
//   0. Clear Previous frame Acceleration. 
//   1. Construct new Acceleration
//   2. Generate a delta movement
//   3. Either apply that delta movement directly to entity or apply it through collision_resolution_move.
static void update_entity_loop(total_entities *global_entities, world global_world, f32 deltaTime, f32 OneSecond) {
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

    // TODO[ECS]: When moving fully to an ECS based look it would be much easier to just be able to 
    // pass in an ID and then get all the needed information from the global entity data.
    //
    v2 entity_movement_delta = generate_delta_movement(global_entities, &global_entities->entities[entity_index], global_world,
                                                       deltaTime);

    collision_resolution_for_move(&global_entities->entities[entity_index], global_world, 
                                  entity_movement_delta, deltaTime);

  }

}
static void render_entity_loop(total_entities *global_entities) {
  for (u8 entity_index = 0; entity_index < global_entities->entity_count; entity_index++) {
    entity currentEntity = global_entities->entities[entity_index];
    switch (currentEntity.state) {
      case IGNORE: {
        DrawRectangleV(Vector2{currentEntity.pos.x, currentEntity.pos.y}, 
                       Vector2{currentEntity.dim.x, currentEntity.dim.y}, RED);
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
