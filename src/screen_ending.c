#include "raylib.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int finishScreen = 0;

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    finishScreen = 0;
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {
        finishScreen = 1;
    }
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);

    Vector2 pos = { 20, 10 };
    DrawTextEx(font, "ENDING SCREEN", pos, font.baseSize * 3.0f, 4, DARKBLUE);
    DrawText("PRESS ENTER or TAP to RETURN to TITLE SCREEN", 120, 220, 20, DARKBLUE);

    if (winner == HUMAN) {
        ClearBackground(RAYWHITE);
        const char* winnerText = "PLAYER WINS!";
        int width = MeasureText(winnerText, 60);
        DrawText(winnerText, GetScreenWidth() / 2 - width / 2, GetScreenHeight() / 2 - 30, 60, BLUE);
    } else if (winner == PC) {
        ClearBackground(RAYWHITE);
        const char* winnerText = "PC WINS!";
        int width = MeasureText(winnerText, 60);
        DrawText(winnerText, GetScreenWidth() / 2 - width / 2, GetScreenHeight() / 2 - 30, 60, RED);
    }
}

// Ending Screen Unload logic
void UnloadEndingScreen(void)
{
}

// Ending Screen should finish?
int FinishEndingScreen(void)
{
    return finishScreen;
}
