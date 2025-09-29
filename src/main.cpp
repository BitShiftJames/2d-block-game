#include "raylib.h"
#include "jamTypes.h"
#include "jamMath.h"


struct tile {
  u32 type;
  // shader information
  u32 light;
};

struct world {
  // maximum world size is 65,535
  u16 Width;
  u16 Height;
  u16 TileSize;
  tile *map;
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

struct entity {
  v2 pos;
  v2 velocity;
  v2 acceleration;
  v2 dim;
};

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
    for (u32 TileY = MinTileY; TileY <= MaxTileY; TileY++) {
      for (u32 TileX = MinTileX; TileX <= MaxTileX; TileX++) {
        // general accessor function incoming.
        tile CurrentTile = global_world.map[TileY * global_world.Width + TileX];

        if (CurrentTile.type != 0) {

          jam_rect2 TileRect = JamRectMinDim(v2{(f32)(TileX * global_world.TileSize), (f32)(TileY * global_world.TileSize)}, 
                                             v2{(f32)global_world.TileSize, (f32)global_world.TileSize});
          // TODO: This could be in a seperate function, maybe.
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

          if (fabsf(Minimum_overlap.x) > fabsf(Minimum_overlap.y)) {
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

          break;

        }

      }
    }

    Entity->pos += delta_movement * (tMin);
    
    f32 normalLength = LengthSq(normal);
    if (normalLength > 0.0f) {
      Entity->velocity = Entity->velocity - 1*Inner(Entity->velocity, normal) * normal;
      // this should entirely remove the problem of sticking I was having in the last version.
      // the problem is back, dumb bass.
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
  entity player_entity = {};
  player_entity.dim.x = 32;
  player_entity.dim.y = 48;
  player_entity.pos = spawn_location;

  f32 inputStrength = 250.0f;

  Camera2D follow_camera = {};
  follow_camera.target = {player_entity.pos.x, player_entity.pos.y};
  follow_camera.zoom = 1.0f;
  follow_camera.offset = {(f32)ScreenWidth / 2, (f32)ScreenHeight / 2};

  f32 Gravity = 9.8;
//f32 OneSecond = 0;
  while (!WindowShouldClose()) {
    f32 deltaTime = GetFrameTime();
 //   OneSecond += deltaTime;

    //   Entity movement will probably look like
    //   0. Clear Previous frame Acceleration.
    //   1. Construct new Acceleration
    //   2. Generate a delta movement
    //   3. Either apply that delta movement directly to entity or apply it through collision_resolution_move.
    //   This means that to do pathing finding it will have to be a solve for acceleration for that frame.
    
    player_entity.acceleration.x = 0;
    player_entity.acceleration.y = 0;
    if (IsKeyDown(KEY_W)) {
      player_entity.acceleration.y--;
    }
    if (IsKeyDown(KEY_A)) {
      player_entity.acceleration.x--;
    }
    if (IsKeyDown(KEY_S)) {
      player_entity.acceleration.y++;
    }
    if (IsKeyDown(KEY_D)) {
      player_entity.acceleration.x++;
    }

    f32 ddPosLength = LengthSq(player_entity.acceleration);
    if (ddPosLength > 1.0f) {
      player_entity.acceleration *= 1.0f / SquareRoot(ddPosLength);
    }
    player_entity.acceleration *= inputStrength;
    player_entity.acceleration += -4.0f * player_entity.velocity;

    v2 entity_delta_movement = generate_delta_movement(player_entity, deltaTime);
    collision_resolution_for_move(&player_entity, global_world, entity_delta_movement, deltaTime);

    follow_camera.target = {player_entity.pos.x, player_entity.pos.y};

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
        

        Rectangle player = {player_entity.pos.x, player_entity.pos.y, player_entity.dim.x, player_entity.dim.y};
        DrawRectangleRec(player, BLUE);
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
