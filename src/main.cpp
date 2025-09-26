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
          CurrentTile.type = 43;
        } else {
          CurrentTile.type = 41;
        }
      }

      global_world.map[tileY * global_world.Width + tileX] = CurrentTile;
    }
  }

  while (!WindowShouldClose()) {

    BeginDrawing();

      ClearBackground(BLACK);

      for (i32 tileY = 0; tileY < global_world.Height; tileY++) {
        for (i32 tileX = 0; tileX < global_world.Width; tileX++) {
          tile CurrentTile = global_world.map[tileY * global_world.Width + tileX];
          // the tiletype of zero in logic is no tile but the tile sheet of 0 is tile 1 so 
          // subtracting one fixes that bias
          i32 ActualTileType = CurrentTile.type - 1;
          if (ActualTileType < 0) {
            continue;
          }
          f32 spacing = (global_world.TileWidth * 3);
          f32 offset = global_world.TileWidth;
          u32 max_x_tile_sheet = (TextileSheet.width - offset) / spacing;
          f32 typeX =  (ActualTileType % max_x_tile_sheet) * spacing + offset;
          f32 typeY =  ((f32)ActualTileType / max_x_tile_sheet) * spacing + offset;
          DrawTextureRec(TextileSheet, 
                         Rectangle{typeX, typeY, (f32)global_world.TileWidth, (f32)global_world.TileHeight}, 
                         Vector2{(f32)(tileX * global_world.TileWidth), (f32)(tileY * global_world.TileHeight)}, WHITE);
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
