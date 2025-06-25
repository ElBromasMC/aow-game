#ifndef SCREENS_H
#define SCREENS_H

//----------------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------------
#include <raylib.h>

//----------------------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------------------
#define MAX_PIECES 18
#define LANE_COUNT 3
#define LANE_WIDTH 4.0f
#define LANE_SPACING 6.0f
#define TARGET_PIECE_HEIGHT 2.5f
#define TARGET_KING_HEIGHT 4.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen {
    UNKNOWN = -1,
    TITLE = 0,
    GAMEPLAY,
    ENDING
} GameScreen;

typedef enum {
    PIECE_PAWN = 0,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_ROOK,
    PIECE_QUEEN
} PieceType;

typedef enum {
    UNDEFINED = -1,
    HUMAN = 0,
    PC
} Winner;

typedef struct Piece {
    PieceType type;
    Vector3 position;
    Vector3 size; // Hitbox size
    int health;
    int maxHealth;
    int damage;
    int cost;
    float speed;
    float attackTimer;
    int active;
    int lane;
} Piece;

typedef struct King {
    Vector3 position;
    int health;
    int maxHealth;
    BoundingBox collisionBox;
} King;

typedef struct Player {
    float points;
    int population;
    King king;
    Piece pieces[MAX_PIECES];
    int capturedPieces[5];
    int isAI;
} Player;

//----------------------------------------------------------------------------------
// Global Variables Declaration (shared by several modules)
//----------------------------------------------------------------------------------
extern GameScreen currentScreen;
extern Model kingModel;
extern Model pieceModels[5];
// extern Texture2D woodTexture;
// extern Texture2D pieceTexture;
extern Music backgroundMusic;
extern Font font;

// Game logic
extern Winner winner;

//----------------------------------------------------------------------------------
// Title Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);
int FinishTitleScreen(void);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitGameplayScreen(void);
void UpdateGameplayScreen(void);
void DrawGameplayScreen(void);
void UnloadGameplayScreen(void);
int FinishGameplayScreen(void);

//----------------------------------------------------------------------------------
// Ending Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitEndingScreen(void);
void UpdateEndingScreen(void);
void DrawEndingScreen(void);
void UnloadEndingScreen(void);
int FinishEndingScreen(void);

#endif // SCREENS_H
