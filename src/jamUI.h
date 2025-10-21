#ifndef JAM_UI_H
#define JAM_UI_H

#include "jamTypes.h"
#include "jamMath.h"
#include "jamInventory.h"
#include "jamCollision.h"

// TODO: See if decoupling some components from jamEntities is worth it.
#include "jamEntities.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define CURSOR_SIDE 32
#define CURSOR_HALF_SIDE 16

#define MAX_HEALTH 400
#define HEALTH_SLOTS 40
#define HALF_HEALTH_SLOTS 20

struct CURSOR_OBJECT {
  Vector2 MousePosition;

  Rectangle layer1;
  Rectangle layer2;
  Rectangle layer3;

  // TODO: Settings.
  Color layer1Tint = {0, 0, 0, 255};
  Color layer2Tint = {255, 255, 255, 255};
  Color layer3Tint = {100, 0, 0, 255};

  Texture2D texture;

  Cursor_inventory_information Inventory;
};

struct UI_ASSETS {
  CURSOR_OBJECT *cursor;

  Inventory_information *playerInventory;
  Inventory_information *storageInventory;

  // TODO: DO something with this.
  healthComponent *bossInformation;
  // TODO: This just being a health component is not speaking to what this 
  // might actually be in future. Spoiler alert it would be easier to change
  // because everything passed to the UI is a pointer and I can change the rendering 
  // logic at any time.
  healthComponent *PlayerInformation;
  
  // TODO: Smarter UI collision.
  u32 player_collision_count;
  u32 storage_collision_count;
  jam_rect2 playerInvCollision[MAX_INVENTORY_SLOTS];
  jam_rect2 StorageInvCollision[MAX_INVENTORY_SLOTS];

  b32 Dirty;
  Texture2D item_icons;
};

void DrawUI(UI_ASSETS *Assets, RenderTexture2D UI_texture, b32 ProfilerToggle, b32 PlayerToggle) {
  TIMED_BLOCK();
    
    if (Assets->Dirty) {
      BeginTextureMode(UI_texture);
        Assets->player_collision_count = 0;
        Assets->storage_collision_count = 0;
        ClearBackground(Color{0, 0, 0, 0});
      
      if (PlayerToggle) {
          for (u32 i = 0; i < Assets->playerInventory->DisplaySlots; i++) {
            u32 slotY = i % Assets->playerInventory->Row;
            u32 slotX = i / Assets->playerInventory->Row;
            
            Rectangle destRect = {(f32)(slotX * (Assets->playerInventory->Size * 1.2) + 20), (f32)(slotY * (Assets->playerInventory->Size * 1.2) + 200), 
                                  (f32)Assets->playerInventory->Size, (f32)Assets->playerInventory->Size};
            
            DrawRectangleRounded(destRect, .3f, 10, Color{100, 0, 255, 128});
            Assets->playerInvCollision[Assets->player_collision_count++] = JamRectMinDim(destRect);
            if (Assets->playerInventory->storage[i].HasItem) {

              DrawTexturePro(Assets->item_icons, JamToRayRect(Assets->playerInventory->storage[i].item_in_me.SourceRect), destRect, Vector2{0, 0}, 0.0f, WHITE);
            }
          }

          for (u32 i = 0; i < Assets->storageInventory->DisplaySlots; i++) {
            u32 slotY = i % Assets->storageInventory->Row;
            u32 slotX = i / Assets->storageInventory->Row;
            
            Rectangle destRect = {(f32)(-(slotX * (Assets->playerInventory->Size * 1.2)) + (GetScreenWidth() - 80)), (f32)(slotY * (Assets->playerInventory->Size * 1.2) + 200), 
                                  (f32)Assets->playerInventory->Size, (f32)Assets->playerInventory->Size};
            DrawRectangleRounded(destRect, .3f, 10, Color{100, 0, 255, 128});
            // USELESS FUCKING LSP.
            Assets->StorageInvCollision[Assets->storage_collision_count++] = JamRectMinDim(destRect);
            if (Assets->storageInventory->storage[i].HasItem) {

              DrawTexturePro(Assets->item_icons, JamToRayRect(Assets->storageInventory->storage[i].item_in_me.SourceRect), destRect, Vector2{0, 0}, 0.0f, WHITE);
            }
          }

          u32 max_count = (u32)(((f32)Assets->PlayerInformation->Health / (f32)MAX_HEALTH) * HEALTH_SLOTS);
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

  DrawTextureRec(Assets->cursor->texture, Assets->cursor->layer1, Assets->cursor->MousePosition, Assets->cursor->layer1Tint);
  DrawTextureRec(Assets->cursor->texture, Assets->cursor->layer2, Assets->cursor->MousePosition, Assets->cursor->layer2Tint);
  DrawTextureRec(Assets->cursor->texture, Assets->cursor->layer3, Assets->cursor->MousePosition, Assets->cursor->layer3Tint);

  if (Assets->cursor->Inventory.storage.HasItem) {
    Rectangle destRect = Rectangle{ Assets->cursor->MousePosition.x - Assets->cursor->Inventory.dest_rect.Max.x, 
                                    Assets->cursor->MousePosition.y - Assets->cursor->Inventory.dest_rect.Max.y, 
                                    Assets->cursor->Inventory.dest_rect.Max.x * 2, 
                                    Assets->cursor->Inventory.dest_rect.Max.y * 2};
    DrawTexturePro(Assets->item_icons, JamToRayRect(Assets->cursor->Inventory.storage.item_in_me.SourceRect), destRect, Vector2{0, 0}, 0.0f, WHITE);
  }
}
#endif
