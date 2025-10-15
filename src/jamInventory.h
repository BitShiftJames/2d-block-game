#ifndef JAM_INVENTORY_H
#define JAM_INVENTORY_H

#include "jamTypes.h"

#define MAX_INVENTORY_SLOTS 40
#define HOT_BAR_SLOTS 8

// Not inlined because it will be used for making a tool to make these item structs from item sheets.
struct item {
  char *Name;

  s32 AtlasIndex;
  jam_rect2 SourceRect;
};

struct Inventory_storage {
  b32 HasItem;
  item item_in_me;
};

struct Inventory_information {
  s32 Row = 8;
  s32 DisplaySlots;
  
  u32 Size = 64;

  Inventory_storage storage[MAX_INVENTORY_SLOTS];
};

struct Cursor_inventory_information {
  // The cursor inventory slot likes to be a different size.
  jam_rect2 dest_rect;
  Inventory_storage storage;
};

void take_from_slot(Inventory_storage *From, Inventory_storage *To) {
      if (From->HasItem) {
        To->HasItem = true;
        To->item_in_me.Name = From->item_in_me.Name;
        To->item_in_me.AtlasIndex = From->item_in_me.AtlasIndex;
        To->item_in_me.SourceRect = From->item_in_me.SourceRect;

        From->HasItem = false;
        From->item_in_me.Name = (char *)"";
        From->item_in_me.AtlasIndex = -1;
        From->item_in_me.SourceRect = {};
      }
}

#endif
