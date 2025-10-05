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
  Color *injectValues = (Color *)malloc(LightTextureSize * LightTextureSize * sizeof(Color));
  Image injectImage = {};
  injectImage.data = injectValues;
  injectImage.width = LightTextureSize;
  injectImage.height = LightTextureSize;
  injectImage.mipmaps = 1;
  injectImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

  Color *propagationValues = (Color *)malloc(LightTextureSize * LightTextureSize * sizeof(Color));
  Image propagationImage = {};
  propagationImage.data = propagationValues;
  propagationImage.width = LightTextureSize;
  propagationImage.height = LightTextureSize;
  propagationImage.mipmaps = 1;
  propagationImage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

  Texture2D LightTexture = LoadTextureFromImage(propagationImage);

  Image blueImage = GenImageColor(LightTextureSize, LightTextureSize, BLUE);
  Texture2D fallBack = LoadTextureFromImage(blueImage);
  UnloadImage(blueImage);

  Shader testShader = LoadShader("../shaders/basic.vert", "../shaders/lighting.frag");
  
  i32 RenderMinimumLoc = GetShaderLocation(testShader, "renderMinimum");
  i32 RenderMaximumLoc = GetShaderLocation(testShader, "renderMaximum");

  i32 LightMaploc = GetShaderLocation(testShader, "texture1");
  f32 LightFallOff = 0.5f;
  
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
      CurrentTile.light = 127;
      if (tileY > global_world.Height / 2) {
        if (tileX % 2 == 1) {
          CurrentTile.type = 42;
          CurrentTile.light = 0;
        } else {
          CurrentTile.type = 43;
          CurrentTile.light = 255;
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

    update_entity_loop(&global_entities, global_world, deltaTime, OneSecond);

    follow_camera.target = {global_entities.entities[entity_id].pos.x, 
                            global_entities.entities[entity_id].pos.y};

    v2 render_distance = {256, 256};
    v2 minimum = {floor_f32((global_entities.entities[entity_id].pos.x / global_world.TileSize) - (render_distance.x / 2)), 
                  floor_f32((global_entities.entities[entity_id].pos.y / global_world.TileSize) - (render_distance.y / 2))};

    minimum.x = Maximum(0, minimum.x);
    minimum.y = Maximum(0, minimum.y);

    jam_rect2 render_rectangle = JamRectMinDim(minimum, render_distance);

    f32 RenderMinimum[2] = {
      render_rectangle.Min.x * global_world.TileSize,
      render_rectangle.Min.y * global_world.TileSize,
    };

    f32 RenderMaximum[2] = {
      render_rectangle.Max.x * global_world.TileSize,
      render_rectangle.Max.y * global_world.TileSize,
    };

    SetShaderValue(testShader, RenderMinimumLoc, RenderMinimum, SHADER_UNIFORM_VEC2);
    SetShaderValue(testShader, RenderMaximumLoc, RenderMaximum, SHADER_UNIFORM_VEC2);

    // new idea seperation of the light pass into two phases one where we inject all the tile light data into the lightmap.
    // and then we do the propagation. Now for propagation we might be lossing the ability to piggy back off the tile light data buffer
    // In a way that allows for only have one buffer of color data, but we should get more verbose behavior when dealing with the light map.
    // Once the propagation is working. There needs to be a check on the possibility of doing just one loop.
    for (u32 LightY = 0; LightY < LightTextureSize; LightY++) {
      for (u32 LightX = 0; LightX < LightTextureSize; LightX++) {
        u32 TileLightY = render_rectangle.Min.y + LightY;
        u32 TileLightX = render_rectangle.Min.x + LightX;
        if (TileLightX > render_rectangle.Max.x || TileLightY > render_rectangle.Max.y) {
          injectValues[LightY * LightTextureSize + LightX] = Color{255, 0, 255, 255};
        } else {

          tile CurrentTile = getTile(global_world, TileLightX, TileLightY);
          injectValues[LightY * LightTextureSize + LightX].r = CurrentTile.light;
          injectValues[LightY * LightTextureSize + LightX].g = CurrentTile.light;
          injectValues[LightY * LightTextureSize + LightX].b = CurrentTile.light;

          injectValues[LightY * LightTextureSize + LightX].r = jamClamp_u8(injectValues[LightY * LightTextureSize + LightX].r, (u8)0, (u8)255);
          injectValues[LightY * LightTextureSize + LightX].g = jamClamp_u8(injectValues[LightY * LightTextureSize + LightX].g, (u8)0, (u8)255);
          injectValues[LightY * LightTextureSize + LightX].b = jamClamp_u8(injectValues[LightY * LightTextureSize + LightX].b, (u8)0, (u8)255);

          // TODO: what can I do with the alpha. In this light stuff
          injectValues[LightY * LightTextureSize + LightX].a = 255;
        }
      }
    }
                
    for (u32 LightY = 0; LightY < LightTextureSize; LightY++) {
      for (u32 LightX = 0; LightX < LightTextureSize; LightX++) {
        // one to one conversion of light values
        Color currentLight = injectValues[LightY * LightTextureSize + LightX];
        // check if current light can propagate.
        propagationValues[LightY * LightTextureSize + LightX] = currentLight;
      }
    }

    UpdateTexture(LightTexture, propagationValues);

    BeginDrawing();
      
      ClearBackground(BLACK);


        BeginMode2D(follow_camera);
          BeginShaderMode(testShader);

          SetShaderValueTexture(testShader, LightMaploc, LightTexture);
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
        DrawRectangle(render_rectangle.Min.x * global_world.TileSize, render_rectangle.Min.y * global_world.TileSize, 40, 40, WHITE);
        DrawRectangle(render_rectangle.Max.x * global_world.TileSize, render_rectangle.Min.y * global_world.TileSize, 40, 40, RED);
        DrawRectangle(render_rectangle.Min.x * global_world.TileSize, render_rectangle.Max.y * global_world.TileSize, 40, 40, BLUE);
        DrawRectangle(render_rectangle.Max.x * global_world.TileSize, render_rectangle.Max.y * global_world.TileSize, 40, 40, PURPLE);

      EndMode2D();


    EndDrawing();

    if (OneSecond >= 1.0f) {
      OneSecond = 0;
    }

  }

  CloseWindow();

  return 0;
}
