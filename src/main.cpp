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
    global_entities.entities[entity_id].acceleration = {0, 0};
    if (IsKeyDown(KEY_W)) {
      global_entities.entities[entity_id].acceleration.y--;
    }
    if (IsKeyDown(KEY_A)) {
      global_entities.entities[entity_id].acceleration.x--;
    }
    if (IsKeyDown(KEY_S)) {
      global_entities.entities[entity_id].acceleration.y++;
    }
    if (IsKeyDown(KEY_D)) {
      global_entities.entities[entity_id].acceleration.x++;
    }

    f32 ddPosLength = LengthSq(global_entities.entities[entity_id].acceleration);
    if (ddPosLength > 1.0f) {
      global_entities.entities[entity_id].acceleration *= 1.0f / SquareRoot(ddPosLength);
    }
    global_entities.entities[entity_id].acceleration *= inputStrength;
    global_entities.entities[entity_id].acceleration += -2.0f * global_entities.entities[entity_id].velocity;

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
