#include "raylib.h"

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define GAME_AREA_WIDTH 160
#define GAME_AREA_HEIGHT 240
#define LEVEL_CELLS_LENGTH 150

/* ---------------------------------- Type ---------------------------------- */
typedef struct Viewport
{
    RenderTexture2D renderTexture2D;
    Rectangle rectSource;
    Rectangle rectDest;
} Viewport;

typedef struct Grid
{
    int width;
    int cells[(GAME_AREA_WIDTH / TILE_WIDTH) * (GAME_AREA_HEIGHT / TILE_HEIGHT)];
} Grid;

typedef struct GameState
{
    Grid level;
} GameState;

/* ----------------------- Local Variables Definition ----------------------- */

Rectangle rect = { 0, 0, 32, 32};

/* ----------------------- Local Function Declaration ----------------------- */

static Viewport ViewportInit(int width, int height, int scale);
static void DrawFrame(Viewport viewport, GameState gamestate, Texture2D tex);        // Update and draw one frame
int GridGetHeight(Grid grid);

int main()
{
    /* ----------------------------- Initialization ----------------------------- */
    int windowWidth = 0;
    int windowHeight = 0;

    /* ------------------------ Window Size and Position ------------------------ */
    InitWindow(100, 100, "Game");
    windowWidth = GAME_AREA_WIDTH * 3 * GetWindowScaleDPI().x;
    windowHeight = GAME_AREA_HEIGHT * 3 * GetWindowScaleDPI().y;
    SetWindowSize(windowWidth, windowHeight);
    SetWindowPosition(GetMonitorWidth(0) / 2 - windowWidth / 2, GetMonitorHeight(0) / 2 - windowHeight / 2);

    Viewport viewport = ViewportInit(GAME_AREA_WIDTH, GAME_AREA_HEIGHT, 3 * GetWindowScaleDPI().x);

    /* ---------------------------- Loading Textures ---------------------------- */
    Texture2D tileset = LoadTexture("data/texture_tileset_01.png");

    /* ----------------------------- Init Game State ---------------------------- */
    GameState gamestate = {0};
    gamestate.level.width = 10;
    for (int i = 0; i < LEVEL_CELLS_LENGTH; i++)
    {
        gamestate.level.cells[i] = 1;
    }

    rect.x = rect.y = 32;

    SetTargetFPS(60);

    /* -------------------------------- Main Loop ------------------------------- */
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        DrawFrame(viewport, gamestate, tileset);
    }

    /* ---------------------------- De-Initialization --------------------------- */
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}

// Update and draw game frame
static void DrawFrame(Viewport viewport, GameState gamestate, Texture2D tex)
{

    /* ---------------------------- Draw in Viewport ---------------------------- */
    BeginTextureMode(viewport.renderTexture2D);
    ClearBackground(DARKGREEN);

    /* ---------------------------------- Grid ---------------------------------- */
    
    for (int i = 0; i < LEVEL_CELLS_LENGTH; i++)
    {
        Rectangle src = {gamestate.level.cells[i] * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};
        Vector2 pos = {i % gamestate.level.width * TILE_WIDTH, i / gamestate.level.width * TILE_HEIGHT};
        DrawTextureRec(tex, src, pos, WHITE);
    }

    EndTextureMode();

    /* ----------------------------- Draw in Window ----------------------------- */
    BeginDrawing();

    ClearBackground(BLACK);

    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(viewport.renderTexture2D.texture, viewport.rectSource, viewport.rectDest, origin, 0.0f, WHITE);

    EndDrawing();
}

static Viewport ViewportInit(int width, int height, int scale)
{
    Rectangle rectSource = {0, 0, width, -height};
    Rectangle rectDest = {0, 0, width * scale, height * scale};
    Viewport viewport = {0};

    viewport.renderTexture2D = LoadRenderTexture(width, height);
    viewport.rectSource = rectSource;
    viewport.rectDest = rectDest;

    return viewport;
}