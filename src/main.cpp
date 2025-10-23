#include "jamInventory.h"
#define PERMISSION 0
#if PERMISSION
// a profiler from Handmade hero (Day 176-179). 
// I got permission but I probably won't include it in the code still.
#include "jamDebug.h"
#else
#define TIMED_BLOCK __noop
#endif

#include "raylib.h"

#include "jamTypes.h"
#include "jamMath.h"
#include "jamEntities.h"
#include "jamTiles.h"
#include "jamLighting.h"

#include "jamUI.h"

#include <string.h>

int main() {
  u32 flags = FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT;// FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED;
  SetConfigFlags(flags);
  // passing 0 makes raylib window the size of the screen.
  InitWindow(0, 0, "Restarting from scratch");
  s32 ScreenWidth = GetScreenWidth();
  s32 ScreenHeight = GetScreenHeight();
  SetTraceLogLevel(LOG_ALL);
  HideCursor();
//  Image bubbleTest = LoadImage("../assets/bubbles.png");
//  Texture2D bubbleTexture = LoadTextureFromImage(bubbleTest);
//  UnloadImage(bubbleTest);
  Image ImgtileSheet = LoadImage("../assets/tilesheet.png");
  Texture2D TextileSheet = LoadTextureFromImage(ImgtileSheet);
  UnloadImage(ImgtileSheet);
  
  SetTextureFilter(TextileSheet, TEXTURE_FILTER_POINT);
  u32 LightTextureDim = 256;
  u32 LightTextureSize = LightTextureDim * LightTextureDim * sizeof(jamColor);

  jamColor *update1Values = (jamColor *)MemAlloc(LightTextureSize);
  Image update1Image = {};
  update1Image.data = update1Values;
  update1Image.width = LightTextureDim;
  update1Image.height = LightTextureDim;
  update1Image.mipmaps = 1;
  update1Image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

  jamColor *update2Values = (jamColor *)MemAlloc(LightTextureSize);
  Image update2Image = {};
  update2Image.data = update2Values;
  update2Image.width = LightTextureDim;
  update2Image.height = LightTextureDim;
  update2Image.mipmaps = 1;
  update2Image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

  // this is so we can see the debug light texture in the top left corner.
  for (u32 i = 0; i < LightTextureDim * LightTextureDim; i++) {
    u32 X = i % LightTextureDim;
    u32 Y = i / LightTextureDim;
    setLightValue(update1Values, LightTextureDim, jamColor{0, 0, 0, 255}, X, Y);
  // only setting the alpha because the debug light texture needs it.
    setLightValue(update2Values, LightTextureDim, jamColor{0, 0, 0, 255}, X, Y);
  }
  
  Texture2D LightTexture = LoadTextureFromImage(update2Image);
  SetTextureFilter(LightTexture, TEXTURE_FILTER_BILINEAR);

  Shader testShader = LoadShader("../shaders/basic.vert", "../shaders/lighting.frag");

  // TODO: Handle screen resize calls.
  RenderTexture2D UI_texture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

  s32 RenderMinimumLoc = GetShaderLocation(testShader, "renderMinimum");
  s32 RenderMaximumLoc = GetShaderLocation(testShader, "renderMaximum");

  s32 LightMaploc = GetShaderLocation(testShader, "texture1");
  f32 LightFallOff = 0.9f;
  UI_ASSETS global_ui_style = {};

  Inventory_information storageInventory = {};
  storageInventory.DisplaySlots = 8;
  storageInventory.Row = 8;
  storageInventory.Size = 64;

  Inventory_information playerInventory = {};
  playerInventory.DisplaySlots = 8;
  playerInventory.Row = 8;
  playerInventory.Size = 64;

  playerInventory.storage[0].HasItem = true;
  playerInventory.storage[0].item_in_me.AtlasIndex = 0;
  playerInventory.storage[0].item_in_me.Name = (char *)"Blue";
  playerInventory.storage[0].item_in_me.SourceRect = JamRectMinDim(v2{0, 0}, v2{16, 16});
  
  // TODO: Scope this.
  CURSOR_OBJECT global_cursor = {};
  global_cursor.texture = LoadTexture("../assets/Cursor.png");

  global_cursor.layer1 = {0, 0, (f32)global_cursor.texture.width, (f32)((u32)(global_cursor.texture.height / 3))};
  global_cursor.layer2 = {0, CURSOR_SIDE, (f32)global_cursor.texture.width, (f32)((u32)(global_cursor.texture.height / 3))};
  global_cursor.layer3 = {0, CURSOR_SIDE * 2, (f32)global_cursor.texture.width, (f32)((u32)(global_cursor.texture.height / 3))};

  Cursor_inventory_information CursorInventory = {};
  CursorInventory.dest_rect = JamRectMinDim(v2{0, 0}, v2{16, 16});

  global_cursor.Inventory = CursorInventory;

  // FIXME: Boss information is not set. Sort of intentional.
  // FIXME: Terrible player naming badness change it to use an entity component of health.
  
  global_ui_style.storageInventory = &storageInventory;
  global_ui_style.playerInventory = &playerInventory;
  global_ui_style.cursor = &global_cursor;


  global_ui_style.Dirty = true;

  // this will have to get a lot smarter in the future.
  global_ui_style.item_icons = LoadTexture("../assets/itemsheet.png");
  SetTextureFilter(global_ui_style.item_icons, TEXTURE_FILTER_POINT);

  world global_world = {0};
  global_world.Width = 500;
  global_world.Height = 300;
  global_world.TileSize = 16;
  global_world.gravity_constant = 50;
  global_world.map = (tile *)MemAlloc(sizeof(tile) * global_world.Width * global_world.Height);
  v2 spawn_location = {};
  for (u32 tileY = 0; tileY < global_world.Height; tileY++) {
    for (u32 tileX = 0; tileX < global_world.Width; tileX++) {
      tile CurrentTile = getTile(global_world, tileX, tileY);
      if (tileY > global_world.Height /2) {
          if (tileX > (global_world.Width / 2) && tileX < (global_world.Width / 2) + 40) {
          } else {
            CurrentTile.type = 43;
          }
          CurrentTile.light = packR4G4B4AF(0, 0, 0);
      } else {
      }

      if (tileY > global_world.Height / 1.5) { 
            CurrentTile.type = 43;
      }

      if (CurrentTile.type == 0) {
        CurrentTile.light = packR4G4B4AF(15, 15, 15);
      }

      if (tileX > global_world.Width - 40) {
        CurrentTile.type = 1;
          CurrentTile.light = packR4G4B4AF(15, 0, 0);
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
  b32 follow = true; 
  total_entities global_entities = {};

  u32 player_entity_count = add_entity(&global_entities, v2{24, 42}, spawn_location, IGNORE);
  
  // TODO[ECS]: There is a lot of badness around the entity count and entity id because they are kind of coupled a lot.
  // At some point they should be fully decoupled.
  
  AddHealthComponent(global_entities.HealthComponents, global_entities.entities[player_entity_count].EntityID, 400);
  AddFallComponent(global_entities.fallComponents, global_entities.entities[player_entity_count].EntityID);
  // This is a lot of indirection to just set the health information but realistically 
  // this is not an operation that is done often.
  healthComponent player_health_component;
  HealthLookUp(global_entities.HealthComponents, &player_health_component, global_entities.entities[player_entity_count].EntityID);
  global_ui_style.PlayerInformation = &player_health_component;

  u32 horse_entity_count = add_entity(&global_entities, v2{60, 42}, spawn_location, IDLE);
  AddHealthComponent(global_entities.HealthComponents, global_entities.entities[horse_entity_count].EntityID, 400);

  b32 profilerToggle = false;
  b32 playerUIToggle = true;

  f32 Gravity = 9.8;
  f32 OneSecond = 0;
  b32 OneOrTwo = 0;
  while (!WindowShouldClose()) {

    f32 deltaTime = GetFrameTime();
    OneSecond += deltaTime;

    if (IsKeyDown(KEY_MINUS) && follow_camera.zoom > .1f) {
      follow_camera.zoom -= .05;
    }
    if (IsKeyDown(KEY_EQUAL) && follow_camera.zoom < 3.0f) {
      follow_camera.zoom += .05;
    }

    if (IsKeyPressed(KEY_F1)) {
      follow = !follow;
    }

    if (IsKeyPressed(KEY_F2)) {
      profilerToggle = !profilerToggle;
    }
    if (IsKeyPressed(KEY_F3)) {
      playerUIToggle = !playerUIToggle;
    }

    if (IsKeyPressed(KEY_TAB)) {
      if (playerInventory.DisplaySlots != 8) {
        playerInventory.DisplaySlots = 8;
        global_ui_style.Dirty = true;       
      } else {
        playerInventory.DisplaySlots = 40;
        global_ui_style.Dirty = true;       
      }
      
    }

    if (follow) {
      update_entity_loop(&global_entities, global_world, deltaTime, OneSecond);
      follow_camera.target = {global_entities.entities[player_entity_count].pos.x, 
                            global_entities.entities[player_entity_count].pos.y};
    } else {
      if (IsKeyDown(KEY_W)) {
        follow_camera.target.y -= 5;
      }
      if (IsKeyDown(KEY_A)) {
        follow_camera.target.x -= 5;
      }
      if (IsKeyDown(KEY_S)) {
        follow_camera.target.y += 5;
      }
      if (IsKeyDown(KEY_D)) {
        follow_camera.target.x += 5;
      }
    }

    Vector2 MousePos = GetMousePosition();
    MousePos.x -= CURSOR_HALF_SIDE;
    MousePos.y -= CURSOR_HALF_SIDE;

    global_cursor.MousePosition = MousePos;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      for (u32 i = 0; i < global_ui_style.player_collision_count; i++) {
        if (PointInRect(global_ui_style.playerInvCollision[i], MousePos)) {
          // Should this also be an inventory function. The line / between UI rendering and logic is hard to make a direct boundary.
          // Respect my boundaries bro.
          // Also right now this is unneeded indirection added for seemingly no benefit. Some benefit in now that I can edit the memory of the inventory out side of UI interaction.
          if (!global_cursor.Inventory.storage.HasItem) {
            take_from_slot(&playerInventory.storage[i], &global_cursor.Inventory.storage);
          } else {
            take_from_slot(&global_cursor.Inventory.storage, &playerInventory.storage[i]);
          }
            
          global_ui_style.Dirty = true;
        }
      }

      // TODO: Collapse this
      if (global_ui_style.storage_collision_count) {
        for (u32 i = 0; i < global_ui_style.storage_collision_count; i++) {
          if (PointInRect(global_ui_style.StorageInvCollision[i], MousePos)) {

            if (!global_cursor.Inventory.storage.HasItem) {
              take_from_slot(&storageInventory.storage[i], &global_cursor.Inventory.storage);
            } else {
              // YOU BETTER SAY BLESS YOU
              take_from_slot(&global_cursor.Inventory.storage, &storageInventory.storage[i]);
            }
              
            global_ui_style.Dirty = true;

          }
        }
      }
    }

    v2 render_distance = {256, 256};
    v2 minimum = {floor_f32((global_entities.entities[player_entity_count].pos.x / global_world.TileSize) - (render_distance.x / 2)), 
                  floor_f32((global_entities.entities[player_entity_count].pos.y / global_world.TileSize) - (render_distance.y / 2))};

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

    
    #if jamLIGHTDEBUB_
      if (IsKeyPressed(KEY_R)) {
        memset(update1Values, 0, LightTextureSize);
        memset(update2Values, 0, LightTextureSize);
      }
      if (IsKeyPressed(KEY_I)) {
        InjectLighting(update1Values, global_world, render_rectangle, LightTextureDim);
      }
      if (IsKeyPressed(KEY_Y)) {
        PropagateLighting(update1Values, update2Values, LightTextureDim);
      }
      if (IsKeyPressed(KEY_U)) {
        PropagateLighting(update2Values, update1Values, LightTextureDim);
      }
      
      if (IsKeyPressed(KEY_COMMA) && lightLookat != 0) {
        lightLookat = 0;
      }

      if (IsKeyPressed(KEY_PERIOD) && lightLookat != 1) {
        lightLookat = 1;
      }

      switch (lightLookat) {
        case (0): {
          UpdateTexture(LightTexture, update1Values);
        } break;
        case (1): {
          UpdateTexture(LightTexture, update2Values);
        } break;
      }
    #else
      memset(update1Values, 0, LightTextureSize);
      memset(update2Values, 0, LightTextureSize);
      
      u32 lightiterCount = 2;
      InjectLighting(update1Values, global_world, global_entities, render_rectangle, LightTextureDim);
      for (u32 iter = 0; iter < lightiterCount; iter++) {
        PropagateLighting(update1Values, update2Values, LightTextureDim);
        PropagateLighting(update2Values, update1Values, LightTextureDim);
      }
      UpdateTexture(LightTexture, update2Values);
    #endif
  

    BeginDrawing();
      
      ClearBackground(SKYBLUE);


        BeginMode2D(follow_camera);
          BeginShaderMode(testShader);

          SetShaderValueTexture(testShader, LightMaploc, LightTexture);
          for (u32 tileY = render_rectangle.Min.y; tileY < render_rectangle.Max.y; tileY++) {
            for (u32 tileX = render_rectangle.Min.x; tileX < render_rectangle.Max.x; tileX++) {
              tile CurrentTile = getTile(global_world, tileX, tileY);
              // the tiletype of zero in logic is no tile but the tile sheet of 0 is tile 1 so 
              // subtracting one fixes that bias
              s32 ActualTileType = CurrentTile.type - 1;
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
        #if 0
          DrawRectangle(render_rectangle.Min.x * global_world.TileSize, render_rectangle.Min.y * global_world.TileSize, 40, 40, WHITE);
          DrawRectangle(render_rectangle.Max.x * global_world.TileSize, render_rectangle.Min.y * global_world.TileSize, 40, 40, RED);
          DrawRectangle(render_rectangle.Min.x * global_world.TileSize, render_rectangle.Max.y * global_world.TileSize, 40, 40, BLUE);
          DrawRectangle(render_rectangle.Max.x * global_world.TileSize, render_rectangle.Max.y * global_world.TileSize, 40, 40, PURPLE);
                 
          for (u32 X = render_rectangle.Min.x; X <= render_rectangle.Max.x; X += 1) {
            DrawText(TextFormat("%u", (X - (u32)render_rectangle.Min.x)), (s32)(X * global_world.TileSize), render_rectangle.Min.y * global_world.TileSize, 1, RED);
            DrawLine(X * global_world.TileSize, render_rectangle.Min.y * global_world.TileSize, X * global_world.TileSize, render_rectangle.Max.y * global_world.TileSize, BLUE);
          }

          for (u32 Y = render_rectangle.Min.y; Y <= render_rectangle.Max.y; Y += 1) {
            DrawText(TextFormat("%u", (Y - (u32)render_rectangle.Min.y)), render_rectangle.Min.x * global_world.TileSize, (s32)(Y * global_world.TileSize) - 8, 1, BLUE);
            DrawLine(render_rectangle.Min.x * global_world.TileSize, Y * global_world.TileSize, render_rectangle.Max.x * global_world.TileSize, Y * global_world.TileSize, RED);
          }
        #endif
    

      EndMode2D();
      DrawText(TextFormat("Fall Velocity %f", global_entities.entities[player_entity_count].velocity.y), 0, 0, 20, WHITE);
      DrawUI(&global_ui_style, UI_texture, profilerToggle, playerUIToggle);
    // TODO: Put this in the DebugUI.
    
    #if 0
      DrawText(TextFormat("Light Texture Look up: %i", lightLookat), 19, 10, 20, WHITE);
      DrawRectangle(17, 37, LightTextureDim + 6, LightTextureDim + 6, WHITE);
      DrawTexture(LightTexture, 20, 40, WHITE);
    #endif
    #if PERMISSION
      Draw_Counter();
    #endif
    EndDrawing();


    if (OneSecond >= 1.0f) {
      OneSecond = 0;
    }

  }

  CloseWindow();
  return 0;
}

#if PERMISSION
  debugRecords MainDebugRecords[__COUNTER__];
  #include "jamDebug.cpp"
#endif
