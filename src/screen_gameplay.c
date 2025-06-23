#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static const int PIECE_STATS[5][3] = {
    { 100, 100, 10 }, // PAWN
    { 200, 150, 20 }, // KNIGHT
    { 250, 120, 25 }, // BISHOP
    { 300, 200, 15 }, // ROOK
    { 500, 350, 40 } // QUEEN
};

// Game entities
static Camera camera = { 0 };
static Player player = { 0 };
static Player computer = { 0 };

// Required variables to manage game logic
static bool showHelp = false;
static bool showHitboxes = false;
static int selectedLane = 0;
static float aiSpawnTimer = 0.0f;
static float kingScale = 0.0f;
static float pieceScales[5] = { 0 };

// Manage game over
static int finishScreen = 0;

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void TrySpawnPiece(Player* p, PieceType type, int lane);
static void HandleInput(void);
static void UpdateAI(void);
static void DrawHealthBar3D(Vector3 position, float modelHeight, int currentHealth, int maxHealth);
static void DrawHelpWindow(void);
static void DrawPieceSelectionUI(void);
static void DrawTexturedPlane(Texture2D texture, Vector2 center, Vector2 size, Vector2 tiling, Color tint);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------
// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    // Camera initialization
    camera.position = (Vector3) { 0.0f, 20.0f, 28.0f };
    camera.target = (Vector3) { 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Player initialization
    player.points = 500.0f;
    player.king.health = 2000;
    player.king.maxHealth = 2000;
    player.king.position = (Vector3) { 0.0f, 0.0f, 20.0f };
    player.king.collisionBox = (BoundingBox) {
        (Vector3) { -LANE_SPACING * 1.5f, 0.0f, player.king.position.z - 2.0f },
        (Vector3) { LANE_SPACING * 1.5f, TARGET_KING_HEIGHT, player.king.position.z + 2.0f }
    };

    // Computer initalization
    computer.points = 500.0f;
    computer.isAI = true;
    computer.king.health = 2000;
    computer.king.maxHealth = 2000;
    computer.king.position = (Vector3) { 0.0f, 0.0f, -20.0f };
    computer.king.collisionBox = (BoundingBox) {
        (Vector3) { -LANE_SPACING * 1.5f, 0.0f, computer.king.position.z - 2.0f },
        (Vector3) { LANE_SPACING * 1.5f, TARGET_KING_HEIGHT, computer.king.position.z + 2.0f }
    };

    // Pieces initialization
    for (int i = 0; i < MAX_PIECES; i++) {
        player.pieces[i].active = false;
        computer.pieces[i].active = false;
    }

    // Game logic initialization
    showHelp = true;
    showHitboxes = false;
    selectedLane = 0;
    aiSpawnTimer = 0.0;
    if (IsModelValid(kingModel)) {
        BoundingBox kingBounds = GetMeshBoundingBox(kingModel.meshes[0]);
        float kingHeight = kingBounds.max.y - kingBounds.min.y;
        if (kingHeight != 0)
            kingScale = TARGET_KING_HEIGHT / kingHeight;
    }
    for (int i = 0; i < 5; i++) {
        if (IsModelValid(pieceModels[i])) {
            BoundingBox bounds = GetMeshBoundingBox(pieceModels[i].meshes[0]);
            float height = bounds.max.y - bounds.min.y;
            if (height != 0)
                pieceScales[i] = TARGET_PIECE_HEIGHT / height;
        }
    }

    // Game over initialization
    finishScreen = 0;
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    HandleInput();
    UpdateAI();

    // Passive points earn for player and computer
    player.points += 2.0f * GetFrameTime();
    computer.points += 4.0f * GetFrameTime();

    // Iterate over both players (player and computer)
    Player* players[2] = { &player, &computer };
    for (int pIdx = 0; pIdx < 2; pIdx++) {
        Player* currentP = players[pIdx];
        Player* opponentP = players[(pIdx + 1) % 2];

        // Movement direction for the pieces
        float moveDirection = currentP->isAI ? 1.0f : -1.0f;

        // Iterate over player's pieces
        for (int i = 0; i < MAX_PIECES; i++) {
            // Check if the piece is active
            if (!currentP->pieces[i].active)
                continue;

            Piece* pPiece = &currentP->pieces[i]; // The current piece
            int isBlocked = 0; // Check if the piece collides with another

            // Construct the hitbox for the piece
            BoundingBox pieceHitbox = {
                .min = { pPiece->position.x - pPiece->size.x / 2, pPiece->position.y, pPiece->position.z - pPiece->size.z / 2 },
                .max = { pPiece->position.x + pPiece->size.x / 2, pPiece->position.y + pPiece->size.y, pPiece->position.z + pPiece->size.z / 2 }
            };

            // Use the hitbox for all collision checks
            if (CheckCollisionBoxes(pieceHitbox, opponentP->king.collisionBox)) {
                // King attack logic
                isBlocked = true;
                pPiece->attackTimer += GetFrameTime();
                if (pPiece->attackTimer >= 1.0f) {
                    pPiece->attackTimer = 0.0f;
                    opponentP->king.health -= pPiece->damage;
                    if (opponentP->king.health < 0)
                        opponentP->king.health = 0;
                    pPiece->health -= 9; // King's damage
                }
            } else {
                for (int j = 0; j < MAX_PIECES; j++) {
                    if (opponentP->pieces[j].active && opponentP->pieces[j].lane == pPiece->lane) {
                        // Construct opponent hitbox
                        Piece* oPiece = &opponentP->pieces[j];
                        BoundingBox opponentHitbox = {
                            .min = { oPiece->position.x - oPiece->size.x / 2, oPiece->position.y, oPiece->position.z - oPiece->size.z / 2 },
                            .max = { oPiece->position.x + oPiece->size.x / 2, oPiece->position.y + oPiece->size.y, oPiece->position.z + oPiece->size.z / 2 }
                        };

                        if (CheckCollisionBoxes(pieceHitbox, opponentHitbox)) {
                            // Opponent piece attack logic
                            isBlocked = true;
                            pPiece->attackTimer += GetFrameTime();
                            if (pPiece->attackTimer >= 1.0f) {
                                pPiece->attackTimer = 0.0f;
                                oPiece->health -= pPiece->damage;
                            }
                            break;
                        }
                    }
                }
            }

            // Collision of pieces of the same team
            if (!isBlocked) {
                for (int j = 0; j < MAX_PIECES; j++) {
                    if (i == j || !currentP->pieces[j].active || currentP->pieces[j].lane != pPiece->lane)
                        continue;
                    int isJInFront = (moveDirection < 0) ? (currentP->pieces[j].position.z < pPiece->position.z) : (currentP->pieces[j].position.z > pPiece->position.z);
                    if (isJInFront && Vector3Distance(pPiece->position, currentP->pieces[j].position) < 2.0f) {
                        isBlocked = true;
                        break;
                    }
                }
            }

            // Piece movement
            if (!isBlocked)
                pPiece->position.z += moveDirection * pPiece->speed * GetFrameTime();

            // Piece death
            if (pPiece->health <= 0) {
                pPiece->active = false;
                currentP->population--;
                // Active points earn
                opponentP->points += pPiece->cost * 1.25f;
            }
        }
    }

    // Check game over
    if (player.king.health <= 0) {
        finishScreen = 1;
        winner = PC;
    }
    if (computer.king.health <= 0) {
        finishScreen = 1;
        winner = HUMAN;
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    ClearBackground(SKYBLUE);
    BeginMode3D(camera);
    DrawTexturedPlane(woodTexture, (Vector2) { 0.0f, 0.0f }, (Vector2) { 50.0f, 50.0f }, (Vector2) { 10.0f, 10.0f }, BROWN);
    // DrawPlane((Vector3) { 0.0f, 0.0f, 0.0f }, (Vector2) { 50.0f, 50.0f }, LIGHTGRAY);
    // DrawGrid(50, 1.0f);

    // Draw Lanes
    for (int i = 1; i <= LANE_COUNT; i++) {
        float laneX = (i - 2) * LANE_SPACING;
        Color laneColor = (selectedLane == i) ? Fade(GOLD, 0.5f) : Fade(DARKGRAY, 0.5f);
        DrawCube((Vector3) { laneX, 0.02f, 0.0f }, LANE_WIDTH, 0.02f, 40.0f, laneColor);
    }

    // Draw Models
    Vector3 kingScaleVec = { 2 * kingScale, 2 * kingScale, 2 * kingScale };
    DrawModelEx(kingModel, player.king.position, (Vector3) { 0, 1, 0 }, 0.0f, kingScaleVec, WHITE);
    DrawModelEx(kingModel, computer.king.position, (Vector3) { 0, 1, 0 }, 180.0f, kingScaleVec, BLACK);

    for (int i = 0; i < MAX_PIECES; i++) {
        if (player.pieces[i].active) {
            Piece* p = &player.pieces[i];
            Vector3 pScale = { pieceScales[p->type], pieceScales[p->type], pieceScales[p->type] };
            DrawModelEx(pieceModels[p->type], p->position, (Vector3) { 0, 1, 0 }, 0.0f, pScale, WHITE);
        }
        if (computer.pieces[i].active) {
            Piece* p = &computer.pieces[i];
            Vector3 pScale = { pieceScales[p->type], pieceScales[p->type], pieceScales[p->type] };
            DrawModelEx(pieceModels[p->type], p->position, (Vector3) { 0, 1, 0 }, 180.0f, pScale, BLACK);
        }
    }

    // Draw Debug Hitboxes (Toggle with 'B')
    if (showHitboxes) {
        DrawBoundingBox(player.king.collisionBox, LIME);
        DrawBoundingBox(computer.king.collisionBox, LIME);

        for (int i = 0; i < MAX_PIECES; i++) {
            if (player.pieces[i].active) {
                Piece* p = &player.pieces[i];
                BoundingBox hitbox = { { p->position.x - p->size.x / 2, p->position.y, p->position.z - p->size.z / 2 },
                    { p->position.x + p->size.x / 2, p->position.y + p->size.y, p->position.z + p->size.z / 2 } };
                DrawBoundingBox(hitbox, Fade(GREEN, 0.5f));
            }
            if (computer.pieces[i].active) {
                Piece* p = &computer.pieces[i];
                BoundingBox hitbox = { { p->position.x - p->size.x / 2, p->position.y, p->position.z - p->size.z / 2 },
                    { p->position.x + p->size.x / 2, p->position.y + p->size.y, p->position.z + p->size.z / 2 } };
                DrawBoundingBox(hitbox, Fade(ORANGE, 0.5f));
            }
        }
    }
    EndMode3D();

    // Draw UI
    DrawHealthBar3D(player.king.position, TARGET_KING_HEIGHT, player.king.health, player.king.maxHealth);
    DrawHealthBar3D(computer.king.position, TARGET_KING_HEIGHT, computer.king.health, computer.king.maxHealth);

    for (int i = 0; i < MAX_PIECES; i++) {
        if (player.pieces[i].active)
            DrawHealthBar3D(player.pieces[i].position, player.pieces[i].size.y, player.pieces[i].health, player.pieces[i].maxHealth);
        if (computer.pieces[i].active)
            DrawHealthBar3D(computer.pieces[i].position, computer.pieces[i].size.y, computer.pieces[i].health, computer.pieces[i].maxHealth);
    }

    DrawRectangle(5, 5, 250, 105, Fade(SKYBLUE, 0.7f));
    DrawRectangleLines(5, 5, 250, 105, BLUE);
    DrawText(TextFormat("Points: %d", (int)player.points), 15, 15, 20, GOLD);
    DrawText(TextFormat("Population: %d/%d", player.population, MAX_PIECES), 15, 40, 20, BLACK);
    DrawText(TextFormat("Selected Lane: %d", selectedLane), 15, 65, 20, (selectedLane > 0) ? LIME : GRAY);
    DrawText("Toggle Hitboxes: [B]", 15, 90, 15, DARKGRAY);
    DrawFPS(GetScreenWidth() - 100, 10);

    DrawPieceSelectionUI();

    if (showHelp)
        DrawHelpWindow();
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Spawns a piece of the given type and in the given lane for the player 'p'
// only if the player have enough points
static void TrySpawnPiece(Player* p, PieceType type, int lane)
{
    if (p->population >= MAX_PIECES)
        return;
    if (p->points < PIECE_STATS[type][0])
        return;

    for (int i = 0; i < MAX_PIECES; i++) {
        if (!p->pieces[i].active) {
            p->points -= PIECE_STATS[type][0];
            p->population++;
            p->pieces[i].active = true;
            p->pieces[i].type = type;
            p->pieces[i].lane = lane;
            // ... (costs, health, etc.)
            p->pieces[i].cost = PIECE_STATS[type][0];
            p->pieces[i].maxHealth = PIECE_STATS[type][1];
            p->pieces[i].health = PIECE_STATS[type][1];
            p->pieces[i].damage = PIECE_STATS[type][2];
            p->pieces[i].speed = 2.5f;
            p->pieces[i].attackTimer = 0.0f;

            // Define the hitbox size for every piece
            p->pieces[i].size = (Vector3) { 1.2f, TARGET_PIECE_HEIGHT, 1.2f };

            float laneX = (lane - 2) * LANE_SPACING;
            float startZ = p->isAI ? -18.0f : 18.0f;
            p->pieces[i].position = (Vector3) { laneX, 0.0f, startZ };
            return;
        }
    }
}

// Handle user input
static void HandleInput(void)
{
    float speed = 12.0f * GetFrameTime();
    float mouseSensibility = 0.1f;
    UpdateCameraPro(&camera,
        (Vector3) {
            (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) * speed - // Move forward-backward
                (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * speed,
            (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * speed - // Move right-left
                (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * speed,
            (IsKeyDown(KEY_SPACE))*speed - // Move up-down
                (IsKeyDown(KEY_LEFT_CONTROL))*speed },
        (Vector3) {
            GetMouseDelta().x * mouseSensibility, // Rotation: yaw
            GetMouseDelta().y * mouseSensibility, // Rotation: pitch
            0.0f // Rotation: roll
        },
        0.0f); // Move to target (zoom)
    if (IsKeyPressed(KEY_B))
        showHitboxes = !showHitboxes;
    if (IsKeyPressed(KEY_H))
        showHelp = !showHelp;
    if (IsKeyPressed(KEY_ONE))
        selectedLane = 1;
    if (IsKeyPressed(KEY_TWO))
        selectedLane = 2;
    if (IsKeyPressed(KEY_THREE))
        selectedLane = 3;
    if (selectedLane != 0) {
        if (IsKeyPressed(KEY_FOUR))
            TrySpawnPiece(&player, PIECE_PAWN, selectedLane);
        if (IsKeyPressed(KEY_FIVE))
            TrySpawnPiece(&player, PIECE_KNIGHT, selectedLane);
        if (IsKeyPressed(KEY_SIX))
            TrySpawnPiece(&player, PIECE_BISHOP, selectedLane);
        if (IsKeyPressed(KEY_SEVEN))
            TrySpawnPiece(&player, PIECE_ROOK, selectedLane);
        if (IsKeyPressed(KEY_EIGHT))
            TrySpawnPiece(&player, PIECE_QUEEN, selectedLane);
    }
}

// Computer actions
static void UpdateAI(void)
{
    aiSpawnTimer += GetFrameTime();
    if (aiSpawnTimer > 2.5f) {
        aiSpawnTimer = (float)GetRandomValue(0, 150) / 100.0f;
        PieceType affordablePieces[5];
        int affordableCount = 0;
        for (int i = 0; i < 5; i++)
            if (computer.points >= PIECE_STATS[i][0])
                affordablePieces[affordableCount++] = (PieceType)i;
        if (affordableCount > 0) {
            int randomLane = GetRandomValue(1, 3);
            PieceType randomPiece = affordablePieces[GetRandomValue(0, affordableCount - 1)];
            TrySpawnPiece(&computer, randomPiece, randomLane);
        }
    }
}

static void DrawHealthBar3D(Vector3 position, float modelHeight, int currentHealth, int maxHealth)
{
    if (currentHealth <= 0)
        return;

    // Health bar 3D position
    Vector3 barPosition = Vector3Add(position, (Vector3) { 0, modelHeight + 0.4f, 0 });

    // Get the vector from the camera to the health bar
    Vector3 toBar = Vector3Subtract(barPosition, camera.position);

    // Get the camera's forward-facing vector
    Vector3 cameraForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    // Check if the health bar is in front of the camera
    float dotProduct = Vector3DotProduct(toBar, cameraForward);

    if (dotProduct > 0) // Only draw if the bar is in front
    {
        // Project the 3D position to 2D screen space
        Vector2 screenPos = GetWorldToScreen(barPosition, camera);

        // Define bar dimensions and calculate health percentage
        float barWidth = 60;
        float barHeight = 8;
        float health = (float)currentHealth / maxHealth;

        // Draw the health bar
        DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth, barHeight, Fade(RED, 0.5f));
        DrawRectangle(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, (int)(barWidth * health), barHeight, GREEN);
        DrawRectangleLines(screenPos.x - barWidth / 2, screenPos.y - barHeight / 2, barWidth, barHeight, DARKGRAY);
    }
}

static void DrawHelpWindow(void)
{
    int width = 500, height = 250, posX = GetScreenWidth() / 2 - width / 2, posY = GetScreenHeight() / 2 - height / 2;
    DrawRectangle(posX, posY, width, height, Fade(RAYWHITE, 0.9f));
    DrawRectangleLines(posX, posY, width, height, DARKGRAY);
    DrawText("GAME CONTROLS (Press H to hide)", posX + 10, posY + 10, 20, BLACK);
    DrawText("Camera: WASD, Space, L-Ctrl, Mouse", posX + 20, posY + 40, 20, DARKGRAY);
    DrawText("Select Lane: Keys 1, 2, 3", posX + 20, posY + 70, 20, DARKGRAY);
    DrawText("Spawn Units (after selecting a lane):", posX + 20, posY + 100, 20, DARKGRAY);
    DrawText("4 - Pawn (100)", posX + 40, posY + 130, 20, DARKGRAY);
    DrawText("5 - Knight (200)", posX + 40, posY + 155, 20, DARKGRAY);
    DrawText("6 - Bishop (250)", posX + 40, posY + 180, 20, DARKGRAY);
    DrawText("7 - Rook (300)", posX + 250, posY + 130, 20, DARKGRAY);
    DrawText("8 - Queen (500)", posX + 250, posY + 155, 20, DARKGRAY);
    DrawText("Objective: Destroy the enemy King!", posX + 20, posY + 215, 20, BLACK);
}

static void DrawPieceSelectionUI(void)
{
    const int panelX = 15;
    const int panelY = 120;
    const int panelWidth = 230;
    const int cardHeight = 60;
    const int spacing = 10;
    const char* pieceNames[] = { "Pawn", "Knight", "Bishop", "Rook", "Queen" };

    for (int i = 0; i < 5; i++) {
        int cardY = panelY + i * (cardHeight + spacing);

        // Card background
        DrawRectangle(panelX, cardY, panelWidth, cardHeight, Fade(RAYWHITE, 0.7f));
        DrawRectangleLines(panelX, cardY, panelWidth, cardHeight, DARKGRAY);

        // Piece Info
        DrawText(TextFormat("%d - %s (%d pts)", i + 4, pieceNames[i], PIECE_STATS[i][0]), panelX + 10, cardY + 5, 20, BLACK);

        // Progress Bar
        int barX = panelX + 10;
        int barY = cardY + 35;
        int barWidth = panelWidth - 20;
        int barHeight = 15;

        // Calculate progress and clamp it between 0 and 1
        float progress = player.points / PIECE_STATS[i][0];
        progress = Clamp(progress, 0.0f, 1.0f);

        bool affordable = (player.points >= PIECE_STATS[i][0]);

        // Draw bar background
        DrawRectangle(barX, barY, barWidth, barHeight, Fade(DARKGRAY, 0.5f));

        // Draw filled part of the bar
        Color barColor = affordable ? GOLD : BLUE;
        DrawRectangle(barX, barY, (int)(barWidth * progress), barHeight, barColor);

        // Draw bar outline
        DrawRectangleLines(barX, barY, barWidth, barHeight, GRAY);
    }
}

static void DrawTexturedPlane(Texture2D texture, Vector2 center, Vector2 size, Vector2 tiling, Color tint)
{
    rlSetTexture(texture.id); // Enable the texture

    rlBegin(RL_QUADS);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0.0f, 1.0f, 0.0f); // All vertices have the same upward-facing normal

    // Vertex 1: Bottom-Left
    // The TexCoord parameters define how the texture maps. (0,0) is one corner, (tiling.x, tiling.y) is the other.
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex3f(center.x - size.x / 2, 0.0f, center.y - size.y / 2);

    // Vertex 2: Top-Left
    rlTexCoord2f(0.0f, tiling.y);
    rlVertex3f(center.x - size.x / 2, 0.0f, center.y + size.y / 2);

    // Vertex 3: Top-Right
    rlTexCoord2f(tiling.x, tiling.y);
    rlVertex3f(center.x + size.x / 2, 0.0f, center.y + size.y / 2);

    // Vertex 4: Bottom-Right
    rlTexCoord2f(tiling.x, 0.0f);
    rlVertex3f(center.x + size.x / 2, 0.0f, center.y - size.y / 2);
    rlEnd();

    rlSetTexture(0); // Disable the texture
}
