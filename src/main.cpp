#include "raylib.h"
#include "jamTypes.h"

int main() {
  i32 ScreenWidth = 800;
  i32 ScreenHeight = 400;

  SetConfigFlags( FLAG_WINDOW_TOPMOST | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
  InitWindow(ScreenWidth, ScreenHeight, "Restarting from scratch");
  SetTargetFPS(60);

  Image bubbleTest = LoadImage("../assets/bubbles.png");
  Texture2D bubbleTexture = LoadTextureFromImage(bubbleTest);
  UnloadImage(bubbleTest);

  // passing in 0 tells raylib to load default.
  Shader testShader = LoadShader(0, "../shaders/lighting.frag");

  while (!WindowShouldClose()) {

    BeginDrawing();

      ClearBackground(GRAY);
      
      DrawText("First window", 30, 30, 20, RED);

        DrawTexture(bubbleTexture, 
                    ScreenWidth / 2 - ((bubbleTexture.width / 2) * 4), 
                    ScreenHeight / 2 - bubbleTexture.height / 2, WHITE);

      BeginShaderMode(testShader);
        DrawTexture(bubbleTexture, 
                    ScreenWidth / 2 - bubbleTexture.width / 2, 
                    ScreenHeight / 2 - bubbleTexture.height / 2, WHITE);
      EndShaderMode();

    EndDrawing();

  }
  CloseWindow();

  return 0;
}
