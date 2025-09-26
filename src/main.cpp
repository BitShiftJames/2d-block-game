#include "raylib.h"
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
  u16 TileWidth;
  u16 TileHeight;
  tile *map;
};

int main() {
  i32 ScreenWidth = 800;
  i32 ScreenHeight = 400;

  SetConfigFlags( FLAG_WINDOW_TOPMOST | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
  InitWindow(ScreenWidth, ScreenHeight, "Restarting from scratch");
  SetTargetFPS(60);

  Image bubbleTest = LoadImage("../assets/bubbles.png");
  Texture2D bubbleTexture = LoadTextureFromImage(bubbleTest);
  UnloadImage(bubbleTest);

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
          CurrentTile.type = 1;
        } else {
          CurrentTile.type = 2;
        }
      }

      global_world.map[tileY * global_world.Width + tileX] = CurrentTile;
    }
  }

  while (!WindowShouldClose()) {

    BeginDrawing();

      ClearBackground(GRAY);

      for (i32 tileY = 0; tileY < global_world.Height; tileY++) {
        for (i32 tileX = 0; tileX < global_world.Width; tileX++) {
          tile CurrentTile = global_world.map[tileY * global_world.Width + tileX];
          switch (CurrentTile.type) {
            case 1: {
              DrawRectangle(tileX * global_world.TileWidth, tileY * global_world.TileHeight, 
                          global_world.TileWidth, global_world.TileHeight, RED);
            } break;
            case 2: {
              DrawRectangle(tileX * global_world.TileWidth, tileY * global_world.TileHeight, 
                          global_world.TileWidth, global_world.TileHeight, BLUE);
            }
          }
        }
      }

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
