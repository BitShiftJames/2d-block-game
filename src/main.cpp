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
  Shader testShader = LoadShader("../shaders/basic.vert", "../shaders/lighting.frag");
  
  i32 lightLoc = GetShaderLocation(testShader, "lightPos");
  i32 lightLoc2 = GetShaderLocation(testShader, "lightPos2");

  int ambientLoc = GetShaderLocation(testShader, "ambient");
  f32 ambientValue[4] = {
    5.0f, 5.0f, 5.0f, 1.0f,
  };
  SetShaderValue(testShader, ambientLoc, ambientValue, SHADER_UNIFORM_VEC4);

  world global_world = {0};
  global_world.Width = 500;
  global_world.Height = 200;
  global_world.TileSize = 16;
  global_world.gravity_constant = 50;
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

    f32 lightPos[3] = {
      global_entities.entities[entity_id].pos.x + (global_entities.entities[entity_id].dim.x / 2), 
      global_entities.entities[entity_id].pos.y + (global_entities.entities[entity_id].dim.y / 2), 
      -20,
    };
    SetShaderValue(testShader, lightLoc, lightPos, SHADER_UNIFORM_VEC3);

    f32 lightPos2[3] = {
      global_entities.entities[horse_id].pos.x + (global_entities.entities[horse_id].dim.x / 2), 
      global_entities.entities[horse_id].pos.y + (global_entities.entities[horse_id].dim.y / 2), 
      -20,
    };
    SetShaderValue(testShader, lightLoc2, lightPos2, SHADER_UNIFORM_VEC3);

    follow_camera.target = {global_entities.entities[entity_id].pos.x, 
                            global_entities.entities[entity_id].pos.y};

    BeginDrawing();
      
      ClearBackground(BLACK);

      BeginShaderMode(testShader);
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
        
        entity_loop(&global_entities, global_world, deltaTime, OneSecond);
      EndShaderMode();

      EndMode2D();

    EndDrawing();

    if (OneSecond >= 1.0f) {
      OneSecond = 0;
    }

  }

  CloseWindow();

  return 0;
}
