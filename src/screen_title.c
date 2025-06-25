#include "raylib.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int finishScreen = 0;
static Camera3D camera = { 0 };
static float rotationY = 0.0f; // Rotation angle for the models
static int framesCounter = 0; // Used for blinking text animation

//----------------------------------------------------------------------------------
// Title Screen Functions Definition
//----------------------------------------------------------------------------------

// Title Screen Initialization logic
void InitTitleScreen(void)
{
    finishScreen = 0;
    framesCounter = 0;
    rotationY = 0.0f;

    // Setup the 3D camera
    camera.position = (Vector3) { 0.0f, 10.0f, 12.0f };
    camera.target = (Vector3) { 0.0f, 3.0f, 0.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

// Title Screen Update logic
void UpdateTitleScreen(void)
{
    // Update animation variables
    framesCounter++;
    rotationY += 0.2f; // Slower rotation speed

    // Continue playing background music
    UpdateMusicStream(backgroundMusic);

    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) {
        finishScreen = 1;
    }
}

// Title Screen Draw logic
void DrawTitleScreen(void)
{
    ClearBackground(BLACK); // A dark background makes the 3D scene pop

    //-- Draw the 3D Scene --//
    BeginMode3D(camera);

    // Draw the King in the center, rotating
    DrawModelEx(kingModel, (Vector3) { 0, 0, 0 }, (Vector3) { 0, 1, 0 }, rotationY, (Vector3) { 1.0f, 1.0f, 1.0f }, WHITE);

    // Draw a few Pawns circling the King
    DrawModelEx(pieceModels[PIECE_PAWN], (Vector3) { -4, 0, 0 }, (Vector3) { 0, 1, 0 }, rotationY, (Vector3) { 1.0f, 1.0f, 1.0f }, WHITE);
    DrawModelEx(pieceModels[PIECE_PAWN], (Vector3) { 4, 0, 0 }, (Vector3) { 0, 1, 0 }, rotationY, (Vector3) { 1.0f, 1.0f, 1.0f }, WHITE);

    // Draw a Knight and a Bishop
    DrawModelEx(pieceModels[PIECE_KNIGHT], (Vector3) { -2, 0, -3 }, (Vector3) { 0, 1, 0 }, rotationY, (Vector3) { 1.0f, 1.0f, 1.0f }, WHITE);
    DrawModelEx(pieceModels[PIECE_BISHOP], (Vector3) { 2, 0, -3 }, (Vector3) { 0, 1, 0 }, rotationY, (Vector3) { 1.0f, 1.0f, 1.0f }, WHITE);

    // Draw a solid-colored plane as the floor instead of a textured model
    DrawPlane((Vector3) { 0.0f, -0.5f, 0.0f }, (Vector2) { 15.0f, 15.0f }, DARKBROWN);
    DrawGrid(20, 2.0f);

    EndMode3D();

    //-- Draw the 2D UI over the 3D scene --//
    // Draw a semi-transparent black rectangle to make text more readable
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));

    // Measure the title text to center it
    const char* titleText = "AOW game";
    Vector2 titleSize = MeasureTextEx(font, titleText, font.baseSize * 3.0f, 4);
    Vector2 titlePos = { (GetScreenWidth() - titleSize.x) / 2.0f, GetScreenHeight() / 2.0f - 100.0f };

    DrawTextEx(font, titleText, titlePos, font.baseSize * 3.0f, 4, GOLD);

    // Draw the blinking prompt text
    if ((framesCounter / 30) % 2) // Toggles every 30 frames (0.5 seconds)
    {
        const char* promptText = "PRESS ENTER or TAP to START";
        Vector2 promptSize = MeasureTextEx(font, promptText, 20, 2);
        Vector2 promptPos = { (GetScreenWidth() - promptSize.x) / 2.0f, GetScreenHeight() / 2.0f + 60.0f };
        DrawTextEx(font, promptText, promptPos, 20, 2, RAYWHITE);
    }
}

// Title Screen Unload logic
void UnloadTitleScreen(void)
{
    // Nothing to unload for this screen now
}

// Title Screen should finish?
int FinishTitleScreen(void)
{
    return finishScreen;
}
