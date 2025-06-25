#include "raylib.h"
#include "raymath.h" // Required for MatrixRotateXYZ
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int finishScreen = 0;
static Camera3D camera = { 0 };
static float rotationY = 0.0f; // Rotation angle for the models
static int framesCounter = 0; // Used for blinking text animation

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    finishScreen = 0;
    framesCounter = 0;
    rotationY = 0.0f;

    // Setup the 3D camera - similar to the title screen
    camera.position = (Vector3) { 0.0f, 12.0f, 14.0f };
    camera.target = (Vector3) { 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Optional: Stop the main gameplay music and play a victory/defeat jingle
    // StopMusicStream(backgroundMusic);
    // PlaySound(fxWinOrLose);
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    // Update animation variables
    framesCounter++;
    rotationY += 0.1f; // A very slow, dramatic rotation

    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {
        finishScreen = 1;
    }
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    ClearBackground(BLACK);

    //-- Prepare Text and Colors based on Winner --//
    const char* winnerText = "";
    Color winnerColor = BLANK;

    if (winner == HUMAN) {
        winnerText = "VICTORY";
        winnerColor = GOLD;
    } else if (winner == PC) {
        winnerText = "DEFEAT";
        winnerColor = MAROON;
    }

    //-- Draw the 3D Scene --//
    BeginMode3D(camera);

    if (winner == HUMAN) {
        // Player wins: Show a triumphant, rotating king
        DrawModelEx(kingModel, (Vector3) { 0, 0, 0 }, (Vector3) { 0, 1, 0 }, rotationY * 5, (Vector3) { 1.2f, 1.2f, 1.2f }, WHITE);
        // Draw some of your pieces celebrating with the king
        DrawModelEx(pieceModels[PIECE_QUEEN], (Vector3) { -4, 0, 0 }, (Vector3) { 0, 1, 0 }, rotationY * 5, (Vector3) { 1.0f, 1.0f, 1.0f }, WHITE);
        DrawModelEx(pieceModels[PIECE_ROOK], (Vector3) { 4, 0, 0 }, (Vector3) { 0, 1, 0 }, rotationY * 5, (Vector3) { 1.0f, 1.0f, 1.0f }, WHITE);
    } else if (winner == PC) {
        // PC wins: Show the player's king, defeated (tipped over and grayed out)
        DrawModelEx(kingModel, (Vector3) { 0, 0, 0 }, (Vector3) { 1, 0, 0 }, 90.0f, (Vector3) { 1.0f, 1.0f, 1.0f }, GRAY);
    }

    DrawPlane((Vector3) { 0.0f, -0.5f, 0.0f }, (Vector2) { 20.0f, 20.0f }, DARKBROWN);
    DrawGrid(20, 2.0f);

    EndMode3D();

    //-- Draw the 2D UI over the 3D scene --//
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));

    // Draw the main result text
    Vector2 winnerSize = MeasureTextEx(font, winnerText, font.baseSize * 4.0f, 5);
    Vector2 winnerPos = { (GetScreenWidth() - winnerSize.x) / 2.0f, GetScreenHeight() / 2.0f - 80.0f };
    DrawTextEx(font, winnerText, winnerPos, font.baseSize * 4.0f, 5, winnerColor);

    // Draw the blinking prompt text
    if ((framesCounter / 30) % 2) {
        const char* promptText = "Press ENTER to return to Title";
        Vector2 promptSize = MeasureTextEx(font, promptText, 20, 2);
        Vector2 promptPos = { (GetScreenWidth() - promptSize.x) / 2.0f, GetScreenHeight() / 2.0f + 40.0f };
        DrawTextEx(font, promptText, promptPos, 20, 2, RAYWHITE);
    }
}

// Ending Screen Unload logic
void UnloadEndingScreen(void)
{
    // Nothing to unload for this screen
}

// Ending Screen should finish?
int FinishEndingScreen(void)
{
    return finishScreen;
}
