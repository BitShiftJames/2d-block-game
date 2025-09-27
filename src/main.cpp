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
  u16 TileWidth;
  u16 TileHeight;
  tile *map;
};

b32 AABBcollisioncheck(Rectangle A, Rectangle B) {
  b32 left = A.x + A.width < B.x;
  b32 right = A.x > B.x + B.width;
  b32 bottom = A.y + A.height < B.y;
  b32 top = A.y > B.height + B.y;
  
  return !(left   ||
           right  ||
           bottom ||
           top);
}

Rectangle rectangle_overlap(Rectangle A, Rectangle B) {
  Rectangle Result = {};
  
  Result.x = B.x - (A.x + A.width);
  Result.width = (B.x + B.width) - A.x;
  Result.y = B.y - (A.y + A.height);
  Result.height = (B.height + B.y) - A.y;
  
  return Result;
}

int main() {
  i32 ScreenWidth = 800;
  i32 ScreenHeight = 400;

  SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
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
  global_world.TileWidth = 16;
  global_world.TileHeight = 16;
  global_world.map = (tile *)MemAlloc(sizeof(tile) * global_world.Width * global_world.Height);
  
  for (i32 tileY = 0; tileY < global_world.Height; tileY++) {
    for (i32 tileX = 0; tileX < global_world.Width; tileX++) {
      tile CurrentTile = global_world.map[tileY * global_world.Width + tileX];
      if (tileY > global_world.TileHeight / 2) {
        if (tileX % 2 == 1) {
          CurrentTile.type = 42;
        } else {
          CurrentTile.type = 43;
        }
      }

      global_world.map[tileY * global_world.Width + tileX] = CurrentTile;
    }
  }

  f32 playerWidth = 32;
  f32 playerHeight = 48;
  v2 playerPos = {};
  v2 Velocity = {};
  f32 inputStrength = 250.0f;

  Rectangle player = {player.x, player.y, playerWidth, playerHeight};
  
  Rectangle test_collision_rect = {player.x + 40, player.y, 64, 64};
  
  Camera2D follow_camera = {};
  follow_camera.target = {player.x, player.y};
  follow_camera.zoom = 1.0f;
  follow_camera.offset = {(f32)ScreenWidth / 2, (f32)ScreenHeight / 2};
  Ray rayw;
  while (!WindowShouldClose()) {
    f32 deltaTime = GetFrameTime();

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
    
    playerPos += (.5 * ddPos * (deltaTime * deltaTime)) + (Velocity * deltaTime);
    Velocity += ddPos * deltaTime;
    

    Rectangle player = {playerPos.x, playerPos.y, playerWidth, playerHeight};
    follow_camera.target = {player.x, player.y};

    Rectangle Overlap = rectangle_overlap(player, test_collision_rect);

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
            f32 spacing = (global_world.TileWidth * 3);
            f32 offset = global_world.TileWidth;
            u32 max_x_tile_sheet = (TextileSheet.width - offset) / spacing;
            f32 typeX =  (ActualTileType % (max_x_tile_sheet)) * spacing + offset;
            f32 typeY =  (floor_f32((f32)ActualTileType / max_x_tile_sheet)) * spacing + offset;
            DrawTextureRec(TextileSheet, 
                           Rectangle{typeX, typeY, (f32)global_world.TileWidth, (f32)global_world.TileHeight}, 
                           Vector2{(f32)(tileX * global_world.TileWidth), (f32)(tileY * global_world.TileHeight)}, WHITE);
          }
        }
        
        Color Rectangle_color = RED;
        if (AABBcollisioncheck(player, test_collision_rect)) {
          Rectangle_color = GREEN;
        }

        DrawRectangleRec(test_collision_rect, Rectangle_color);
        DrawRectangleRec(player, BLUE);
      EndMode2D();
      DrawText(TextFormat("Left: %02.02f, Right: %02.02f\nTop: %02.02f, Bottom: %02.02f", Overlap.x, Overlap.width, Overlap.y, Overlap.height), 20, 20, 20, WHITE);
// shader work later
//      BeginShaderMode(testShader);
//        DrawTexture(bubbleTexture, 
//                    ScreenWidth / 2 - bubbleTexture.width / 2, 
//                    ScreenHeight / 2 - bubbleTexture.height / 2, WHITE);
//      EndShaderMode();

    EndDrawing();

  }
  CloseWindow();

  return 0;
}
