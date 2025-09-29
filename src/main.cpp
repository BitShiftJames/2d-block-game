#include "raylib.h"
#include "jamTypes.h"
#include "jamMath.h"
#include "jamEntities.h"
#include "jamTiles.h"


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

int main() {
  i32 ScreenWidth = 800;
  i32 ScreenHeight = 400;
  u32 flags = FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT;// FLAG_WINDOW_TOPMOST;
  SetConfigFlags(flags);
  InitWindow(ScreenWidth, ScreenHeight, "Restarting from scratch");
  SetTargetFPS(60);
  
//  Image bubbleTest = LoadImage("../assets/bubbles.png");
//  Texture2D bubbleTexture = LoadTextureFromImage(bubbleTest);
//  UnloadImage(bubbleTest);
  Image ImgtileSheet = LoadImage("../assets/tilesheet.png");
  Texture2D TextileSheet = LoadTextureFromImage(ImgtileSheet);
  UnloadImage(ImgtileSheet);
// passing in 0 tells raylib to load default. 
  Shader testShader = LoadShader(0, "../shaders/lighting.frag");

  world global_world = {0};
  global_world.Width = 500;
  global_world.Height = 200;
  global_world.TileSize = 16;
  global_world.map = (tile *)MemAlloc(sizeof(tile) * global_world.Width * global_world.Height);
  v2 spawn_location = {};
  for (i32 tileY = 0; tileY < global_world.Height; tileY++) {
    for (i32 tileX = 0; tileX < global_world.Width; tileX++) {
      tile CurrentTile = global_world.map[tileY * global_world.Width + tileX];
      if (tileY > global_world.Height / 2) {
        if (tileX % 2 == 1) {
          CurrentTile.type = 42;
        } else {
          CurrentTile.type = 43;
        }

      }

      if (tileX > global_world.Width - 40) {
        CurrentTile.type = 1;
      }

      if (tileX == global_world.Width / 2 && tileY == (global_world.Height / 2) - 3) {
        spawn_location = {(f32)tileX * global_world.TileSize, (f32)tileY * global_world.TileSize};
      }

      global_world.map[tileY * global_world.Width + tileX] = CurrentTile;
    }
  }

  f32 inputStrength = 250.0f;


  Camera2D follow_camera = {};
  follow_camera.target = {spawn_location.x, spawn_location.y};
  follow_camera.zoom = 1.0f;
  follow_camera.offset = {(f32)ScreenWidth / 2, (f32)ScreenHeight / 2};
  
  total_entities global_entities = {};

  u8 entity_id = add_entity(&global_entities, v2{24, 42}, spawn_location, IGNORE, true);

  f32 Gravity = 9.8;
//f32 OneSecond = 0;
  while (!WindowShouldClose()) {
    f32 deltaTime = GetFrameTime();
 //   OneSecond += deltaTime;

    //   Entity movement will probably look like
    //   0. Clear Previous frame Acceleration. That will not be done at the end of frame.
    //   any entity that changes their acceleration should be responsible for clearing that same
    //   acceleration.
    //   1. Construct new Acceleration
    //   2. Generate a delta movement
    //   3. Either apply that delta movement directly to entity or apply it through collision_resolution_move.
    //   This means that to do pathing finding it will have to be a solve for acceleration for that frame.
    player_acceleration(&global_entities.entities[entity_id], inputStrength);

    v2 player_movement_delta = generate_delta_movement(global_entities.entities[entity_id], 
                                                       deltaTime);

    collision_resolution_for_move(&global_entities.entities[entity_id], global_world, 
                                  player_movement_delta, deltaTime);
    //global_entities.entities[entity_id].pos += player_movement_delta;
    //global_entities.entities[entity_id].velocity += global_entities.entities[entity_id].acceleration * deltaTime;
    
    follow_camera.target = {global_entities.entities[entity_id].pos.x, 
                            global_entities.entities[entity_id].pos.y};

    BeginDrawing();
      
      ClearBackground(BLACK);

      BeginMode2D(follow_camera);
        for (i32 tileY = 0; tileY < global_world.Height; tileY++) {
          for (i32 tileX = 0; tileX < global_world.Width; tileX++) {
            tile CurrentTile = global_world.map[tileY * global_world.Width + tileX];
            // the tiletype of zero in logic is no tile but the tile sheet of 0 is tile 1 so 
            // subtracting one fixes that bias
            i32 ActualTileType = CurrentTile.type - 1;
            if (ActualTileType < 0) {
              continue;
            }
            // some of these can be done in u32 we do not need decimal precision. For this.
            f32 spacing = (global_world.TileSize * 3);
            f32 offset = global_world.TileSize;
            u32 max_x_tile_sheet = (TextileSheet.width - offset) / spacing;
            f32 typeX =  (ActualTileType % (max_x_tile_sheet)) * spacing + offset;
            f32 typeY =  (floor_f32((f32)ActualTileType / max_x_tile_sheet)) * spacing + offset;
            DrawTextureRec(TextileSheet, 
                           Rectangle{typeX, typeY, (f32)global_world.TileSize, (f32)global_world.TileSize}, 
                           Vector2{(f32)(tileX * global_world.TileSize), (f32)(tileY * global_world.TileSize)}, WHITE);
          }
        }
        
        entity_loop(&global_entities);

      EndMode2D();
// shader work later
//      BeginShaderMode(testShader);
//        DrawTexture(bubbleTexture, 
//                    ScreenWidth / 2 - bubbleTexture.width / 2, 
//                    ScreenHeight / 2 - bubbleTexture.height / 2, WHITE);
//      EndShaderMode();

    EndDrawing();

//    if (OneSecond >= 1.0f) {
//      OneSecond = 0;
//    }

  }
  CloseWindow();

  return 0;
}
