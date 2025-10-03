#include "raylib.h"
#include "jamTypes.h"
#include "jamMath.h"
#include "jamEntities.h"
#include "jamTiles.h"

int main() {
  i32 ScreenWidth = 800;
  i32 ScreenHeight = 400;
  u32 flags = FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT;// FLAG_WINDOW_TOPMOST;
  SetConfigFlags(flags);
  InitWindow(ScreenWidth, ScreenHeight, "Restarting from scratch");
  SetTraceLogLevel(LOG_ALL);
  SetTargetFPS(60);
  
//  Image bubbleTest = LoadImage("../assets/bubbles.png");
//  Texture2D bubbleTexture = LoadTextureFromImage(bubbleTest);
//  UnloadImage(bubbleTest);
  Image ImgtileSheet = LoadImage("../assets/tilesheet.png");
  Texture2D TextileSheet = LoadTextureFromImage(ImgtileSheet);
  UnloadImage(ImgtileSheet);
  
  u32 LightTextureSize = 256;
  Color *lightValues = (Color *)malloc(LightTextureSize * LightTextureSize * sizeof(Color));
  Image lightImage = {};
  lightImage.data = lightValues;
  lightImage.width = LightTextureSize;
  lightImage.height = LightTextureSize;
  lightImage.mipmaps = 1;
  lightImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

  Texture2D LightTexture = LoadTextureFromImage(lightImage);
  
  Shader testShader = LoadShader("../shaders/basic.vert", "../shaders/lighting.frag");
  
  i32 RenderInformationLoc = GetShaderLocation(testShader, "render_distance_information");
  i32 LightMaploc = GetShaderLocation(testShader, "lightMap");
  
  SetShaderValueTexture(testShader, LightMaploc, LightTexture);

  world global_world = {0};
  global_world.Width = 500;
  global_world.Height = 200;
  global_world.TileSize = 16;
  global_world.gravity_constant = 50;
  global_world.map = (tile *)MemAlloc(sizeof(tile) * global_world.Width * global_world.Height);
  v2 spawn_location = {};
  for (u32 tileY = 0; tileY < global_world.Height; tileY++) {
    for (u32 tileX = 0; tileX < global_world.Width; tileX++) {
      tile CurrentTile = getTile(global_world, tileX, tileY);
      if (tileY > global_world.Height / 2) {
        if (tileX % 2 == 1) {
          CurrentTile.type = 42;
          CurrentTile.light = 0.5f;
        } else {
          CurrentTile.type = 43;
          CurrentTile.light = 0.25f;
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



  Camera2D follow_camera = {};
  follow_camera.target = {spawn_location.x, spawn_location.y};
  follow_camera.zoom = 1.0f;
  follow_camera.offset = {(f32)ScreenWidth / 2, (f32)ScreenHeight / 2};
  
  total_entities global_entities = {};

  u8 entity_id = add_entity(&global_entities, v2{24, 42}, spawn_location, IGNORE, true);
  u8 horse_id = add_entity(&global_entities, v2{60, 42}, spawn_location, IDLE, true);

  f32 Gravity = 9.8;
  f32 OneSecond = 0;
  while (!WindowShouldClose()) {
    f32 deltaTime = GetFrameTime();
    OneSecond += deltaTime;

    if (IsKeyDown(KEY_MINUS) && follow_camera.zoom > .1f) {
      follow_camera.zoom -= .05;
    }
    if (IsKeyDown(KEY_EQUAL) && follow_camera.zoom < 3.0f) {
      follow_camera.zoom += .05;
    }

    follow_camera.target = {global_entities.entities[entity_id].pos.x, 
                            global_entities.entities[entity_id].pos.y};

    update_entity_loop(&global_entities, global_world, deltaTime, OneSecond);

    v2 render_distance = {200, 115};
    v2 minimum = {floor_f32((global_entities.entities[entity_id].pos.x / global_world.TileSize) - (render_distance.x / 2)), 
                  floor_f32((global_entities.entities[entity_id].pos.y / global_world.TileSize) - (render_distance.y / 2))};

    jam_rect2 render_rectangle = JamRectMinDim(minimum, render_distance);

    f32 Render_distance_values[4] = {
      render_rectangle.Min.x,
      render_rectangle.Min.y,
      render_rectangle.Max.x,
      render_rectangle.Max.y,
    };

    SetShaderValue(testShader, RenderInformationLoc, Render_distance_values, SHADER_UNIFORM_VEC4);

    for (u32 tileY = render_rectangle.Min.y; tileY < render_rectangle.Max.y; tileY++) { 
      for (u32 tileX = render_rectangle.Min.x; tileX < render_rectangle.Max.x; tileX++) {
        // map tiles to light map.
          tile CurrentTile = getTile(global_world, tileX, tileY);

          u32 lightX = (u32)((f32)tileX / (f32)render_rectangle.Max.x) * 256.0f;
          u32 lightY = (u32)((f32)tileY / (f32)render_rectangle.Max.y) * 256.0f;
          u8 color_value = (u8)(CurrentTile.light * 255);

          lightValues[lightY * LightTextureSize + lightX] = Color{color_value, color_value, color_value, 255};
      }
    }

    UpdateTexture(LightTexture, lightValues);

    BeginDrawing();
      
      ClearBackground(BLACK);

      BeginShaderMode(testShader);
      BeginMode2D(follow_camera);
        for (u32 tileY = render_rectangle.Min.y; tileY < render_rectangle.Max.y; tileY++) {
          for (u32 tileX = render_rectangle.Min.x; tileX < render_rectangle.Max.x; tileX++) {
            tile CurrentTile = getTile(global_world, tileX, tileY);
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
        
        render_entity_loop(&global_entities);
      EndShaderMode();

      DrawRectangle(render_rectangle.Min.x * global_world.TileSize, render_rectangle.Min.y * global_world.TileSize, 5, 5, WHITE);
      DrawRectangle(render_rectangle.Max.x * global_world.TileSize, render_rectangle.Min.y * global_world.TileSize, 5, 5, RED);
      DrawRectangle(render_rectangle.Min.x * global_world.TileSize, render_rectangle.Max.y * global_world.TileSize, 5, 5, BLUE);
      DrawRectangle(render_rectangle.Max.x * global_world.TileSize, render_rectangle.Max.y * global_world.TileSize, 5, 5, PURPLE);
      EndMode2D();
    EndDrawing();

    if (OneSecond >= 1.0f) {
      OneSecond = 0;
    }

  }

  CloseWindow();

  return 0;
}
