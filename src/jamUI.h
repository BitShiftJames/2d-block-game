#ifndef JAM_UI_H
#define JAM_UI_H

#include "jamTypes.h"
#include "jamMath.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define CURSOR_SIDE 32
#define CURSOR_HALF_SIDE 16

#define MAX_HEALTH 400
#define HEALTH_SLOTS 40
#define HALF_HEALTH_SLOTS 20

struct CURSOR_OBJECT {
  Rectangle layer1;
  Rectangle layer2;
  Rectangle layer3;

  // TODO: Settings.
  Color layer1Tint = {0, 0, 0, 255};
  Color layer2Tint = {255, 255, 255, 255};
  Color layer3Tint = {100, 0, 0, 255};

  Texture2D texture;
};

struct Inventory_information {
  // TODO: Somewhere else.
  s32 Width = 8;
  s32 DisplaySlots;

  u32 Size = 64;
};

// TODO: Could this be in a better place.
struct player_information {
  u32 EffectBitField;
  u32 Health;
};

struct boss_information {
  u32 Health;
};

struct UI_ASSETS {
  CURSOR_OBJECT cursor;
  Inventory_information Playerinventory;
  player_information PlayerInformation;

  Inventory_information storageInventory;
  // TODO: DO something with this.
  boss_information bossInformation;

  // supports 32 UI containers.
  b32 Dirty;
};

void InitUI(UI_ASSETS *Assets) {
  Texture2D CursorTex = LoadTexture("../assets/Cursor.png");

  Assets->cursor.layer1 = {0, 0, (f32)CursorTex.width, (f32)((u32)(CursorTex.height / 3))};
  Assets->cursor.layer2 = {0, CURSOR_SIDE, (f32)CursorTex.width, (f32)((u32)(CursorTex.height / 3))};
  Assets->cursor.layer3 = {0, CURSOR_SIDE * 2, (f32)CursorTex.width, (f32)((u32)(CursorTex.height / 3))};

  Assets->cursor.texture = CursorTex;

  Assets->Playerinventory.DisplaySlots = 8;
  Assets->storageInventory.DisplaySlots = 0;

  Assets->PlayerInformation.Health = 358;

  Assets->Dirty = true;
}
void DrawUI(UI_ASSETS *Assets, RenderTexture2D UI_texture, b32 ProfilerToggle, b32 PlayerToggle) {
  TIMED_BLOCK();

    if (Assets->Dirty) {
      BeginTextureMode(UI_texture);
        ClearBackground(Color{0, 0, 0, 0});
      // 3, 000k cycles at peak with compiler optimizations turned off.
      // 2, 000k cycles at peak with compiler optimizations turned onn.
      // TODO: Make a container thing where each element is a rendertexture2D so that it doesn't matter.
      // If an element static or constistently changes. <--- is this truly needed the function is performant enough to not matter.
      
      if (ProfilerToggle) {
        Rectangle panel = {30, 30, 300, 500};
        
        DrawRectangleGradientEx(panel, Color{0, 0, 0, 127}, Color{0, 0, 0, 100}, Color{0, 0, 0, 68}, Color{0, 0, 0, 100});
        const char *Title = "Profiler Results: ";
        DrawText(Title, ((panel.x + panel.width / 2) - (s32)(MeasureText(Title, 20) / 2)), panel.y + 20, 20, Color{40, 20, 20, 255});
        DrawLine(panel.x, panel.y + 60, panel.x + panel.width, panel.y + 60, WHITE);
      }

      if (PlayerToggle) {
        
          for (u32 i = 0; i < Assets->Playerinventory.DisplaySlots; i++) {
            u32 slotY = i % Assets->Playerinventory.Width;
            u32 slotX = i / Assets->Playerinventory.Width;

            DrawRectangleRounded(Rectangle{(f32)(slotX * (Assets->Playerinventory.Size * 1.2) + 20), (f32)(slotY * (Assets->Playerinventory.Size * 1.2) + 200), 
                                           (f32)Assets->Playerinventory.Size, (f32)Assets->Playerinventory.Size}, .3f, 10, Color{100, 0, 255, 128});
          }

          for (u32 i = 0; i < Assets->storageInventory.DisplaySlots; i++) {
            u32 slotY = i % Assets->storageInventory.Width;
            u32 slotX = i / Assets->storageInventory.Width;

            DrawRectangleRounded(Rectangle{(f32)(-(slotX * (Assets->Playerinventory.Size * 1.2)) + (GetScreenWidth() - 80)), (f32)(slotY * (Assets->Playerinventory.Size * 1.2) + 200), 
                                           (f32)Assets->Playerinventory.Size, (f32)Assets->Playerinventory.Size}, .3f, 10, Color{100, 0, 255, 128});
          }

          u32 max_count = (u32)(((f32)Assets->PlayerInformation.Health / (f32)MAX_HEALTH) * HEALTH_SLOTS);
          for (u32 i = 0; i < max_count; i++) {
            u32 Y = i / HALF_HEALTH_SLOTS;
            u32 X = i % HALF_HEALTH_SLOTS;
            DrawRectangle( 30 + (X * 27), (Y * 30) + 30, 25, 20, RED);
          }
        
      }

      Assets->Dirty = false;
      EndTextureMode();
    }


  Vector2 MousePos = GetMousePosition();
  MousePos.x -= CURSOR_HALF_SIDE;
  MousePos.y -= CURSOR_HALF_SIDE;

  DrawTextureRec(Assets->cursor.texture, Assets->cursor.layer1, MousePos, Assets->cursor.layer1Tint);
  DrawTextureRec(Assets->cursor.texture, Assets->cursor.layer2, MousePos, Assets->cursor.layer2Tint);
  DrawTextureRec(Assets->cursor.texture, Assets->cursor.layer3, MousePos, Assets->cursor.layer3Tint);

  DrawTexturePro(UI_texture.texture, Rectangle{0.0f, 0.0f, (f32)UI_texture.texture.width, -(f32)UI_texture.texture.height}, Rectangle{0.0f, 0.0f, (f32)UI_texture.texture.width, (f32)UI_texture.texture.height}, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}
#endif
