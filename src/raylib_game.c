#include "raylib.h"
#include "screens.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
// NOTE: Those variables are shared between modules through screens.h
//----------------------------------------------------------------------------------
GameScreen currentScreen = TITLE;
Model kingModel = { 0 };
Model pieceModels[5] = { 0 };
Texture2D woodTexture = { 0 };
Texture2D pieceTexture = { 0 };
Music backgroundMusic = { 0 };
Font font = { 0 };

Winner winner = UNDEFINED;

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 1280;
static const int screenHeight = 720;
// Required variables to manage screen transitions (fade-in, fade-out)
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static GameScreen transToScreen = UNKNOWN;

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void TransitionToScreen(int screen); // Request transition to next screen
static void UpdateTransition(void); // Update transition effect
static void DrawTransition(void); // Draw transition effect (full-screen rectangle)

static void UpdateDrawFrame(void); // Update and draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "AOW Game");
    InitAudioDevice();

    currentScreen = TITLE;
    kingModel = LoadModel("resources/models/king.glb");
    pieceModels[PIECE_PAWN] = LoadModel("resources/models/pawn.glb");
    pieceModels[PIECE_KNIGHT] = LoadModel("resources/models/knight.glb");
    pieceModels[PIECE_BISHOP] = LoadModel("resources/models/bishop.glb");
    pieceModels[PIECE_ROOK] = LoadModel("resources/models/rook.glb");
    pieceModels[PIECE_QUEEN] = LoadModel("resources/models/queen.glb");
    woodTexture = LoadTexture("resources/images/wood.png");
    pieceTexture = LoadTexture("resources/images/piece.png");
    backgroundMusic = LoadMusicStream("resources/audio/background.ogg");
    font = LoadFont("resources/images/mecha.png");

    SetMusicVolume(backgroundMusic, 1.0f);
    PlayMusicStream(backgroundMusic);

    kingModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = woodTexture;
    for (int i = 0; i < 5; i++) {
        pieceModels[i].materials[0].maps[MATERIAL_MAP_ALBEDO].texture = pieceTexture;
    }

    DisableCursor(); // Limit cursor to relative movement inside the window

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(kingModel);
    for (int i = 0; i < 5; i++)
        UnloadModel(pieceModels[i]);
    UnloadTexture(woodTexture);
    UnloadMusicStream(backgroundMusic);

    CloseAudioDevice();
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Request transition to next screen
static void TransitionToScreen(int screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

// Update transition effect (fade-in, fade-out)
static void UpdateTransition(void)
{
    if (!transFadeOut) {
        transAlpha += 0.05f;

        // NOTE: Due to float internal representation, condition jumps on 1.0f instead of 1.05f
        // For that reason we compare against 1.01f, to avoid last frame loading stop
        if (transAlpha > 1.01f) {
            transAlpha = 1.0f;

            // Unload current screen
            switch (transFromScreen) {
            case TITLE:
                UnloadTitleScreen();
                break;
            case GAMEPLAY:
                UnloadGameplayScreen();
                break;
            case ENDING:
                UnloadEndingScreen();
                break;
            default:
                break;
            }

            // Load next screen
            switch (transToScreen) {
            case TITLE:
                InitTitleScreen();
                break;
            case GAMEPLAY:
                InitGameplayScreen();
                break;
            case ENDING:
                InitEndingScreen();
                break;
            default:
                break;
            }

            currentScreen = transToScreen;

            // Activate fade out effect to next loaded screen
            transFadeOut = true;
        }
    } else // Transition fade out logic
    {
        transAlpha -= 0.02f;

        if (transAlpha < -0.01f) {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = UNKNOWN;
        }
    }
}

// Draw transition effect (full-screen rectangle)
static void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------

    UpdateMusicStream(backgroundMusic);
    if (!onTransition) {
        switch (currentScreen) {
        case TITLE: {
            UpdateTitleScreen();
            if (FinishTitleScreen())
                TransitionToScreen(GAMEPLAY);
        } break;
        case GAMEPLAY: {
            UpdateGameplayScreen();
            if (FinishGameplayScreen())
                TransitionToScreen(ENDING);
        } break;
        case ENDING: {
            UpdateEndingScreen();
            if (FinishEndingScreen())
                TransitionToScreen(TITLE);
        } break;
        default:
            break;
        }
    } else
        UpdateTransition(); // Update transition (fade-in, fade-out)
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    switch (currentScreen) {
    case TITLE:
        DrawTitleScreen();
        break;
    case GAMEPLAY:
        DrawGameplayScreen();
        break;
    case ENDING:
        DrawEndingScreen();
        break;
    default:
        break;
    }

    // Draw full screen rectangle in front of everything
    if (onTransition)
        DrawTransition();

    EndDrawing();
    //----------------------------------------------------------------------------------
}
