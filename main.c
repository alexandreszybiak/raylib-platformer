#include "raylib.h"

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define GAME_AREA_WIDTH 160
#define GAME_AREA_HEIGHT 240
#define LEVEL_CELLS_LENGTH 150
#define LEVEL_CELLS_LAST_INDEX 149
#define PIXEL_SIZE 3

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

typedef struct EditorState
{
    int cursorX;
    int cursorY;
    Texture2D selector;
} EditorState;



/* ----------------------- Local Variables Definition ----------------------- */

Rectangle rect = { 0, 0, 32, 32};

/* ----------------------- Local Function Declaration ----------------------- */

static Viewport ViewportInit(int width, int height, int scale);
static void DrawViewport(Viewport viewport, GameState gamestate, Texture2D tex);        // Update and draw one frame
static void DrawEditorUI(EditorState editorState);
static void DrawWorld(Viewport vp, GameState state, Texture2D tex);
int GridGetHeight(Grid grid);
static void GridSet(Grid *grid, int value, int x, int y);

int main()
{
    /* ----------------------------- Initialization ----------------------------- */
    int windowWidth = 0;
    int windowHeight = 0;

    /* ------------------------ Window Size and Position ------------------------ */
    InitWindow(100, 100, "Game");
    windowWidth = GAME_AREA_WIDTH * PIXEL_SIZE * GetWindowScaleDPI().x;
    windowHeight = GAME_AREA_HEIGHT * PIXEL_SIZE * GetWindowScaleDPI().y;
    SetWindowSize(windowWidth, windowHeight);
    SetWindowPosition(GetMonitorWidth(0) / 2 - windowWidth / 2, GetMonitorHeight(0) / 2 - windowHeight / 2);

    Viewport viewport = ViewportInit(GAME_AREA_WIDTH, GAME_AREA_HEIGHT, 3 * GetWindowScaleDPI().x);

    /* ---------------------------- Loading Textures ---------------------------- */
    Texture2D tileset = LoadTexture("data/texture_tileset_01.png");
    Texture2D selector = LoadTexture("data/texture_ui_selector.png");

    /* ----------------------------- Init Game State ---------------------------- */
    GameState gamestate = {0};
    gamestate.level.width = 10;
    for (int i = 0; i < LEVEL_CELLS_LENGTH; i++)
    {
        gamestate.level.cells[i] = 1;
    }

    /* ---------------------------- Init Editor State --------------------------- */
    EditorState editorState = {0};
    editorState.cursorX = editorState.cursorY = 0;
    editorState.selector = selector;

    rect.x = rect.y = 32;

    SetTargetFPS(60);

    /* -------------------------------- Main Loop ------------------------------- */
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        /* --------------------------------- Inputs --------------------------------- */
        if(IsKeyPressed(KEY_ENTER))
        {
            GridSet(&gamestate.level, 0, editorState.cursorX, editorState.cursorY);
        }
        editorState.cursorX += IsKeyPressed(KEY_RIGHT) + -IsKeyPressed(KEY_LEFT);
        editorState.cursorY += IsKeyPressed(KEY_DOWN) + -IsKeyPressed(KEY_UP);

        /* ---------------------------------- Draw ---------------------------------- */
        BeginTextureMode(viewport.renderTexture2D);
        DrawWorld(viewport, gamestate, tileset);
        DrawEditorUI(editorState);
        EndTextureMode();

        BeginDrawing();
        DrawViewport(viewport, gamestate, tileset);
        EndDrawing();
    }

    /* ---------------------------- De-Initialization --------------------------- */
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}

// Update and draw game frame
static void DrawViewport(Viewport viewport, GameState gamestate, Texture2D tex)
{
    /* ------------------------- Draw Viewport in Window ------------------------ */
    ClearBackground(BLACK);

    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(viewport.renderTexture2D.texture, viewport.rectSource, viewport.rectDest, origin, 0.0f, WHITE);
}

static void DrawWorld(Viewport vp, GameState state, Texture2D tex)
{    
    ClearBackground(DARKGREEN);

    /* ---------------------------------- Grid ---------------------------------- */
    for (int i = 0; i < LEVEL_CELLS_LENGTH; i++)
    {
        Rectangle src = {state.level.cells[i] * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};
        Vector2 pos = {i % state.level.width * TILE_WIDTH, i / state.level.width * TILE_HEIGHT};
        DrawTextureRec(tex, src, pos, WHITE);
    }
}

static void DrawEditorUI(EditorState editorState)
{   
    DrawTexture(editorState.selector, editorState.cursorX * TILE_WIDTH, editorState.cursorY * TILE_HEIGHT, GREEN);
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

static void GridSet(Grid *grid, int value, int x, int y)
{
    if(x < 0 || x >= grid->width) return;
    if(y < 0 || y >= GridGetHeight(*grid)) return;
    grid->cells[y * grid->width + x] = value;
}

int GridGetHeight(Grid grid)
{
    return LEVEL_CELLS_LENGTH / grid.width;
}