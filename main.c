#include <math.h>
#include "raylib.h"

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define LEVEL_WIDTH 8
#define LEVEL_HEIGHT 8
#define LEVEL_CELLS_LENGTH LEVEL_WIDTH * LEVEL_HEIGHT
#define GAME_AREA_WIDTH TILE_WIDTH * LEVEL_WIDTH
#define GAME_AREA_HEIGHT TILE_HEIGHT * LEVEL_HEIGHT
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
    int cells[LEVEL_CELLS_LENGTH];
} Grid;

typedef struct Player
{
    Rectangle rect;
    Vector2 velocity;
    Vector2 movementRemainder;
} Player;

typedef struct GameState
{
    Grid level;
    Player player;
} GameState;

typedef struct EditorState
{
    int cursorX;
    int cursorY;
    Texture2D selector;
    bool active;
} EditorState;



/* ----------------------- Local Variables Definition ----------------------- */



/* ----------------------- Local Function Declaration ----------------------- */

/* -------------------------------- Utilities ------------------------------- */
int signf(float f);
int signf(float f)
{
    if (f > 0) return 1;
    if (f < 0) return -1;
    return 0;
}

const Viewport ViewportInit(int width, int height, int scale);

static void DrawViewport(Viewport viewport, GameState gamestate, Texture2D tex);        // Update and draw one frame
static void DrawEditorUI(EditorState editorState);
static void DrawWorld(Viewport vp, GameState state, Texture2D tex);

static void SaveLevel(const Grid *grid);
static void LoadLevel(Grid *grid);

const int GridGetHeight(Grid grid);
static void GridSet(Grid *grid, int value, int x, int y);
const int GridGet(const Grid *grid, int x, int y);
const bool CheckCollisionGridRec(const Grid *grid, Rectangle rect);
const bool CheckCollisionGridPoint(const Grid *grid, int x, int y);

static void PlayerMoveX(GameState *gamestate, float amount);
static void PlayerMoveY(GameState *gamestate, float amount);
const bool PlayerCollideSolid(const Player *player, Grid grid);

Camera2D worldSpaceCamera = { 0 };  // Game world camera
Camera2D screenSpaceCamera = { 0 }; // Smoothing camera

int main()
{
    /* ----------------------------- Initialization ----------------------------- */
    int windowWidth = TILE_WIDTH * LEVEL_WIDTH * PIXEL_SIZE;
    int windowHeight = 0;

    /* ------------------------ Window Size and Position ------------------------ */
    //SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(100, 100, "Game");
    windowWidth = GAME_AREA_WIDTH * PIXEL_SIZE * GetWindowScaleDPI().x;
    windowHeight = GAME_AREA_HEIGHT * PIXEL_SIZE * GetWindowScaleDPI().y;
    SetWindowSize(windowWidth, windowHeight);
    SetWindowPosition(GetMonitorWidth(0) / 2 - windowWidth / 2, GetMonitorHeight(0) / 2 - windowHeight / 2);

    Viewport viewport = ViewportInit(GAME_AREA_WIDTH, GAME_AREA_HEIGHT, 3 * GetWindowScaleDPI().x);

    worldSpaceCamera.zoom = 1.0f;
    screenSpaceCamera.zoom = 1.0f;

    /* ---------------------------- Loading Textures ---------------------------- */
    Texture2D tileset = LoadTexture("data/texture_tileset_01.png");
    Texture2D selector = LoadTexture("data/texture_ui_selector.png");

    /* ----------------------------- Init Game State ---------------------------- */
    GameState gamestate = {0};
    gamestate.level.width = LEVEL_WIDTH;
    for (int i = 0; i < LEVEL_CELLS_LENGTH; i++)
    {
        gamestate.level.cells[i] = 1;
    }
    LoadLevel(&gamestate.level);
    gamestate.player.rect.x = gamestate.player.rect.y = TILE_WIDTH+2;
    gamestate.player.rect.width = 14;
    gamestate.player.rect.height = 26;
    gamestate.player.velocity.x = gamestate.player.velocity.y = 0.0f;

    /* ---------------------------- Init Editor State --------------------------- */
    EditorState editorState = {0};
    editorState.cursorX = editorState.cursorY = 0;
    editorState.selector = selector;
    editorState.active = false;

    /* -------------------------------- Main Loop ------------------------------- */
    SetTargetFPS(60);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        /* --------------------------------- Inputs --------------------------------- */
        if(editorState.active == true)
        {
            if(IsKeyDown(KEY_LEFT_CONTROL))
            {
                if(IsKeyPressed(KEY_S))
                {
                    SaveLevel(&gamestate.level);
                }
            }
            if(IsKeyPressed(KEY_SPACE))
            {   
                int value = !GridGet(&gamestate.level, editorState.cursorX, editorState.cursorY);
                GridSet(&gamestate.level, value, editorState.cursorX, editorState.cursorY);
            }
            editorState.cursorX += IsKeyPressed(KEY_RIGHT) + -IsKeyPressed(KEY_LEFT);
            editorState.cursorY += IsKeyPressed(KEY_DOWN) + -IsKeyPressed(KEY_UP);
            if(IsKeyPressed(KEY_P)) editorState.active = false;
        }
        
        else
        {
            int moveCommand = IsKeyDown(KEY_RIGHT) + -IsKeyDown(KEY_LEFT);
            int jumpCommand = IsKeyPressed(KEY_UP);

            /* ---------------------------- Game State Update --------------------------- */
            gamestate.player.velocity.y += jumpCommand * -4.0f;
            gamestate.player.velocity.x = moveCommand * 2;
            gamestate.player.velocity.y += 0.2f;
            PlayerMoveX(&gamestate, gamestate.player.velocity.x);
            PlayerMoveY(&gamestate, gamestate.player.velocity.y);
            if(IsKeyPressed(KEY_P)) editorState.active = true;
        }

        /* ---------------------------------- Draw ---------------------------------- */
        BeginTextureMode(viewport.renderTexture2D);
        BeginMode2D(worldSpaceCamera);
        DrawWorld(viewport, gamestate, tileset);
        EndMode2D();
        if(editorState.active) DrawEditorUI(editorState);
        EndTextureMode();

        BeginDrawing();
        BeginMode2D(screenSpaceCamera);
        DrawViewport(viewport, gamestate, tileset);
        DrawFPS(GetScreenWidth() - 95, 10);
        EndMode2D();
        EndDrawing();
    }

    /* ---------------------------- De-Initialization --------------------------- */
    UnloadRenderTexture(viewport.renderTexture2D);
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

    /* ------------------------------- Draw Player ------------------------------ */
    DrawRectangleRec(state.player.rect, WHITE);
}

static void DrawEditorUI(EditorState editorState)
{   
    DrawTexture(editorState.selector, editorState.cursorX * TILE_WIDTH, editorState.cursorY * TILE_HEIGHT, GREEN);
}

const Viewport ViewportInit(int width, int height, int scale)
{
    Rectangle rectSource = {0, 0, width, -height};
    Rectangle rectDest = {0, 0, width * scale, height * scale};
    Viewport viewport = {0};

    viewport.renderTexture2D = LoadRenderTexture(width, height);
    viewport.rectSource = rectSource;
    viewport.rectDest = rectDest;

    return viewport;
}

const int GridGet(const Grid *grid, int x, int y)
{
    if(x < 0 || x >= grid->width) return 0;
    if(y < 0 || y >= GridGetHeight(*grid)) return 0;
    return grid->cells[y * grid->width + x];
}

static void GridSet(Grid *grid, int value, int x, int y)
{
    if(x < 0 || x >= grid->width) return;
    if(y < 0 || y >= GridGetHeight(*grid)) return;
    grid->cells[y * grid->width + x] = value;
}

const int GridGetHeight(Grid grid)
{
    return LEVEL_CELLS_LENGTH / grid.width;
}

const bool CheckCollisionGridPoint(const Grid *grid, int x, int y)
{
    if(GridGet(grid, x / TILE_WIDTH, y / TILE_HEIGHT) > 0) return true;
    return false;
}

const bool CheckCollisionGridRec(const Grid *grid, Rectangle rect)
{
    if(CheckCollisionGridPoint(grid, rect.x, rect.y))                                       return true;
    if(CheckCollisionGridPoint(grid, rect.x + rect.width - 1, rect.y))                      return true;
    if(CheckCollisionGridPoint(grid, rect.x + rect.width - 1, rect.y + rect.height - 1))    return true;
    if(CheckCollisionGridPoint(grid, rect.x, rect.y + rect.height - 1))                     return true;
    return false;
}

static void SaveLevel(const Grid *grid)
{
    SaveFileData("level.bin", (void*)grid->cells, sizeof(grid->cells));
    return;
}

static void LoadLevel(Grid *grid)
{
    int dataSize = 0;

    if(!FileExists("level.bin")) return;
    unsigned char *data = 0;
    data = LoadFileData("level.bin", &dataSize);

    for(int i = 0; i < dataSize; i += 4)
    {
        grid->cells[i / 4] = *(data + i);
    }
    unsigned char c = *(data + 8);

       

    UnloadFileData(data);
}

static void PlayerMoveX(GameState *gamestate, float amount)
{
    gamestate->player.movementRemainder.x += amount;
    int move = round(gamestate->player.movementRemainder.x);
    if(move == 0) return;
    gamestate->player.movementRemainder.x -= move;
    int dir = signf(move);
    while(move != 0)
    {
        Rectangle rect = gamestate->player.rect;
        rect.x += dir;
        if(CheckCollisionGridRec(&gamestate->level, rect))
        {
            gamestate->player.velocity.x = 0;
            break;
        }
        gamestate->player.rect.x += dir;
        move -= dir;
    }

    return;
}

static void PlayerMoveY(GameState *gamestate, float amount)
{
    gamestate->player.movementRemainder.y += amount;
    int move = round(gamestate->player.movementRemainder.y);
    if(move == 0) return;
    gamestate->player.movementRemainder.y -= move;
    int dir = signf(move);
    while(move != 0)
    {
        Rectangle rect = gamestate->player.rect;
        rect.y += dir;
        if(CheckCollisionGridRec(&gamestate->level, rect))
        {
            gamestate->player.velocity.y = 0;
            break;
        }
        gamestate->player.rect.y += dir;
        move -= dir;
    }

    return;
}