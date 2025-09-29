#include "raylib.h"
#include "jamTypes.h"
#include "jamMath.h"
#include <complex>

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

jam_rect2 rectangle_overlap(jam_rect2 A, jam_rect2 B) {
  jam_rect2 Result = {};
  
  Result.x = B.x - A.Max.x;
  Result.Max.x = B.Max.x - A.x;
  Result.y = B.y - A.Max.y;
  Result.Max.y = B.Max.y - A.y;
  
  return Result;
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

  f32 playerWidth = 32;
  f32 playerHeight = 48;
  v2 playerPos = spawn_location;
  v2 Velocity = {};
  f32 inputStrength = 250.0f;

  Camera2D follow_camera = {};
  follow_camera.target = {playerPos.x, playerPos.y};
  follow_camera.zoom = 1.0f;
  follow_camera.offset = {(f32)ScreenWidth / 2, (f32)ScreenHeight / 2};
//f32 OneSecond = 0;
  while (!WindowShouldClose()) {
    f32 deltaTime = GetFrameTime();
 //   OneSecond += deltaTime;

    v2 ddPos = {};
    if (IsKeyDown(KEY_W)) {
      ddPos.y--;
    }
    if (IsKeyDown(KEY_A)) {
      ddPos.x--;
    }
    if (IsKeyDown(KEY_S)) {
      ddPos.y++;
    }
    if (IsKeyDown(KEY_D)) {
      ddPos.x++;
    }

    f32 ddPosLength = LengthSq(ddPos);
    if (ddPosLength > 1.0f) {
      ddPos *= 1.0f / SquareRoot(ddPosLength);
    }

    ddPos *= inputStrength;

    ddPos += -4.0f * Velocity;
    v2 delta_movement = (.5 * ddPos * (deltaTime * deltaTime)) + (Velocity * deltaTime);
    v2 new_position = playerPos + delta_movement;
    v2 Min = {Minimum(playerPos.x, new_position.x), Minimum(playerPos.y, new_position.y)};
    v2 Max = {Maximum(playerPos.x + playerWidth, new_position.x + playerWidth), Maximum(playerPos.y + playerHeight, new_position.y + playerHeight)};
    jam_rect2 player_collision_rect = JamRectMinMax(Min, Max);

    u32 MinTileX = (floor_f32(Min.x)) / global_world.TileSize;
    u32 MinTileY = (floor_f32(Min.y)) / global_world.TileSize;
    u32 MaxTileX = (floor_f32(Max.x)) / global_world.TileSize;
    u32 MaxTileY = (floor_f32(Max.y)) / global_world.TileSize;

    f32 tMin = 1.0f;
    v2 normal = {};
    for (u32 TileY = MinTileY; TileY <= MaxTileY; TileY++) {
      for (u32 TileX = MinTileX; TileX <= MaxTileX; TileX++) {
        // general accessor function incoming.
        tile CurrentTile = global_world.map[TileY * global_world.Width + TileX];

        if (CurrentTile.type != 0) {

          v2 TileMin = {(f32)(TileX * global_world.TileSize), (f32)(TileY * global_world.TileSize)}; 
          v2 TileDim = {(f32)global_world.TileSize, (f32)global_world.TileSize};

          jam_rect2 TileRect = JamRectMinDim(TileMin, TileDim);
          
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

    playerPos += delta_movement * (tMin);
    
    f32 normalLength = LengthSq(normal);
    if (normalLength > 0.0f) {
      Velocity = Velocity - 1*Inner(Velocity, normal) * normal;
      // this should entirely remove the problem of sticking I was having in the last version.
      ddPos = ddPos - Inner(ddPos, normal) * normal;
    } else {
    }

    Velocity += ddPos * deltaTime;
    
    follow_camera.target = {playerPos.x, playerPos.y};


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
        

        Rectangle player = {playerPos.x, playerPos.y, playerWidth, playerHeight};
        DrawRectangleRec(player, BLUE);
      EndMode2D();
      DrawText(TextFormat("MinTile: (%u, %u)\nMaxTile: (%u, %u)\nPlayer Position (%f, %f)", MinTileX, MinTileY, MaxTileX, MaxTileY, playerPos.x, playerPos.y), 20, 20, 20, WHITE);
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
