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

#define MAX_INVENTORY_SLOTS 40
#define HOT_BAR_SLOTS 8
inline b32 PointInRect(Rectangle A, Vector2 Point) {
  b32 withinX = Point.x > A.x && Point.x < A.x + A.width;
  b32 withinY = Point.y > A.y && Point.y < A.y + A.height;
  
  b32 within = withinX && withinY;

  return within;
}
struct item {
  char *Name;

  s32 AtlasIndex;
  Rectangle SourceRect;

  // Do I want to support item resizing.
  // Rectangle DestRect;
};

struct CURSOR_OBJECT {
  Rectangle layer1;
  Rectangle layer2;
  Rectangle layer3;

  // TODO: Settings.
  Color layer1Tint = {0, 0, 0, 255};
  Color layer2Tint = {255, 255, 255, 255};
  Color layer3Tint = {100, 0, 0, 255};

  Texture2D texture;

  b32 HasItem;
  item item_in_me;
};


struct Inventory_storage {
  // maybe add slot color to this or something.
  b32 HasItem;
  item item_in_me;
};

struct Inventory_information {
  // TODO: Somewhere else.
  s32 Width = 8;
  s32 DisplaySlots;
  
  u32 Size = 64;

  Inventory_storage storage[MAX_INVENTORY_SLOTS];
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
  
  // Quite literally the stupidest thing I am doing.
  u32 player_collision_count;
  u32 storage_collision_count;
  Rectangle playerInvCollision[MAX_INVENTORY_SLOTS];
  Rectangle StorageInvCollision[MAX_INVENTORY_SLOTS];
  // supports 32 UI containers.
  b32 Dirty;

  Texture2D item_icons;
};

// TODO: Decouple inventory informatiom from the UI. This should just be the needed render information inventory.
// It would also make it easier for the UI to handle it's own dirty check. If items had their own logical information. 
void InitUI(UI_ASSETS *Assets) {
  Assets->cursor.texture = LoadTexture("../assets/Cursor.png");

  Assets->cursor.layer1 = {0, 0, (f32)Assets->cursor.texture.width, (f32)((u32)(Assets->cursor.texture.height / 3))};
  Assets->cursor.layer2 = {0, CURSOR_SIDE, (f32)Assets->cursor.texture.width, (f32)((u32)(Assets->cursor.texture.height / 3))};
  Assets->cursor.layer3 = {0, CURSOR_SIDE * 2, (f32)Assets->cursor.texture.width, (f32)((u32)(Assets->cursor.texture.height / 3))};

  Assets->Playerinventory.DisplaySlots = 8;
  // hard coding it for now.
  // TODO: Make a tool for creating Item information.
  Assets->Playerinventory.storage[0].HasItem = true;
  Assets->Playerinventory.storage[0].item_in_me.Name = (char *)("Blue");
  Assets->Playerinventory.storage[0].item_in_me.AtlasIndex = 0;
  Assets->Playerinventory.storage[0].item_in_me.SourceRect = {0, 0, 16, 16};

  Assets->Playerinventory.storage[1].HasItem = true;
  Assets->Playerinventory.storage[1].item_in_me.Name = (char *)("Blue");
  Assets->Playerinventory.storage[1].item_in_me.AtlasIndex = 0;
  Assets->Playerinventory.storage[1].item_in_me.SourceRect = {0, 0, 16, 16};

  Assets->Playerinventory.storage[8].HasItem = true;
  Assets->Playerinventory.storage[8].item_in_me.Name = (char *)("Blue");
  Assets->Playerinventory.storage[8].item_in_me.AtlasIndex = 0;
  Assets->Playerinventory.storage[8].item_in_me.SourceRect = {0, 0, 16, 16};

  Assets->storageInventory.DisplaySlots = 0;

  Assets->PlayerInformation.Health = 358;

  Assets->Dirty = true;

  // this will have to get a lot smarter in the future.
  Assets->item_icons = LoadTexture("../assets/itemsheet.png");
  SetTextureFilter(Assets->item_icons, TEXTURE_FILTER_POINT);

}

void DrawUI(UI_ASSETS *Assets, RenderTexture2D UI_texture, b32 ProfilerToggle, b32 PlayerToggle) {
  TIMED_BLOCK();
    
    if (Assets->Dirty) {
      BeginTextureMode(UI_texture);
        Assets->player_collision_count = 0;
        Assets->storage_collision_count = 0;
        ClearBackground(Color{0, 0, 0, 0});
      // 3, 000k cycles at peak with compiler optimizations turned off.
      // 2, 000k cycles at peak with compiler optimizations turned onn.
      // TODO: Make a container thing where each element is a rendertexture2D so that it doesn't matter.
      // If an element static or constistently changes. <--- is this truly needed the function is performant enough to not matter.
      
      if (PlayerToggle) {
          for (u32 i = 0; i < Assets->Playerinventory.DisplaySlots; i++) {
            u32 slotY = i % Assets->Playerinventory.Width;
            u32 slotX = i / Assets->Playerinventory.Width;
            
            Rectangle destRect = {(f32)(slotX * (Assets->Playerinventory.Size * 1.2) + 20), (f32)(slotY * (Assets->Playerinventory.Size * 1.2) + 200), 
                                  (f32)Assets->Playerinventory.Size, (f32)Assets->Playerinventory.Size};
            
            DrawRectangleRounded(destRect, .3f, 10, Color{100, 0, 255, 128});
            Assets->playerInvCollision[Assets->player_collision_count++] = destRect;
            if (Assets->Playerinventory.storage[i].HasItem) {

              DrawTexturePro(Assets->item_icons, Assets->Playerinventory.storage[i].item_in_me.SourceRect, destRect, Vector2{0, 0}, 0.0f, WHITE);
            }
          }

          for (u32 i = 0; i < Assets->storageInventory.DisplaySlots; i++) {
            u32 slotY = i % Assets->storageInventory.Width;
            u32 slotX = i / Assets->storageInventory.Width;
            
            Rectangle destRect = {(f32)(-(slotX * (Assets->Playerinventory.Size * 1.2)) + (GetScreenWidth() - 80)), (f32)(slotY * (Assets->Playerinventory.Size * 1.2) + 200), 
                                  (f32)Assets->Playerinventory.Size, (f32)Assets->Playerinventory.Size};
            DrawRectangleRounded(destRect, .3f, 10, Color{100, 0, 255, 128});
            Assets->StorageInvCollision[Assets->storage_collision_count++] = destRect;
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

  DrawTexturePro(UI_texture.texture, Rectangle{0.0f, 0.0f, (f32)UI_texture.texture.width, -(f32)UI_texture.texture.height}, Rectangle{0.0f, 0.0f, (f32)UI_texture.texture.width, (f32)UI_texture.texture.height}, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

  Vector2 MousePos = GetMousePosition();
  MousePos.x -= CURSOR_HALF_SIDE;
  MousePos.y -= CURSOR_HALF_SIDE;
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    for (u32 i = 0; i < Assets->player_collision_count; i++) {
      Rectangle CollisionRect = Assets->playerInvCollision[i];
      if (PointInRect(CollisionRect, MousePos)) {
        if (!Assets->cursor.HasItem) {
          Assets->cursor.HasItem = true;
          Assets->cursor.item_in_me.Name = Assets->Playerinventory.storage[i].item_in_me.Name;
          Assets->cursor.item_in_me.AtlasIndex = Assets->Playerinventory.storage[i].item_in_me.AtlasIndex;
          Assets->cursor.item_in_me.SourceRect = Assets->Playerinventory.storage[i].item_in_me.SourceRect;

          Assets->Playerinventory.storage[i].HasItem = false;
          Assets->Playerinventory.storage[i].item_in_me.Name = (char *)"";
          Assets->Playerinventory.storage[i].item_in_me.AtlasIndex = -1;
          Assets->Playerinventory.storage[i].item_in_me.SourceRect = {};
        } else {

          Assets->Playerinventory.storage[i].HasItem = true;
          Assets->Playerinventory.storage[i].item_in_me.Name = Assets->cursor.item_in_me.Name;
          Assets->Playerinventory.storage[i].item_in_me.AtlasIndex = Assets->cursor.item_in_me.AtlasIndex;
          Assets->Playerinventory.storage[i].item_in_me.SourceRect = Assets->cursor.item_in_me.SourceRect;

          Assets->cursor.HasItem = false;
          Assets->cursor.item_in_me.Name = (char *)"";
          Assets->cursor.item_in_me.AtlasIndex = -1;
          Assets->cursor.item_in_me.SourceRect = {};
        }
          
        Assets->Dirty = true;
      }
    }
    // TODO: Collapse this
    if (Assets->storage_collision_count) {
      for (u32 i = 0; i < Assets->storage_collision_count; i++) {
        Rectangle CollisionRect = Assets->StorageInvCollision[i];
        if (PointInRect(CollisionRect, MousePos)) {
          Assets->storageInventory.storage[i].HasItem = false;
          Assets->Dirty = true;
        }
      }
    }
  }
  DrawTextureRec(Assets->cursor.texture, Assets->cursor.layer1, MousePos, Assets->cursor.layer1Tint);
  DrawTextureRec(Assets->cursor.texture, Assets->cursor.layer2, MousePos, Assets->cursor.layer2Tint);
  DrawTextureRec(Assets->cursor.texture, Assets->cursor.layer3, MousePos, Assets->cursor.layer3Tint);

  if (Assets->cursor.HasItem) {
    Rectangle destRect = Rectangle{ MousePos.x - Assets->cursor.item_in_me.SourceRect.width, 
                                    MousePos.y - Assets->cursor.item_in_me.SourceRect.height, 
                                    Assets->cursor.item_in_me.SourceRect.width * 2, 
                                    Assets->cursor.item_in_me.SourceRect.height * 2};
    DrawTexturePro(Assets->item_icons, Assets->cursor.item_in_me.SourceRect, destRect, Vector2{0, 0}, 0.0f, WHITE);
  }
}
#endif
