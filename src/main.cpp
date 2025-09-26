#include "raylib.h"
#include "jamTypes.h"

int main() {
  i32 ScreenWidth = 800;
  i32 ScreenHeight = 400;

  SetConfigFlags( FLAG_WINDOW_TOPMOST | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
  InitWindow(ScreenWidth, ScreenHeight, "Restarting from scratch");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    BeginDrawing();

      ClearBackground(GRAY);
      
      DrawText("First window", ScreenWidth/2 - 50, ScreenHeight/2, 20, RED);

    EndDrawing();

  }
  CloseWindow();

  return 0;
}
