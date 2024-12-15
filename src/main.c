#include <math.h>
#include "raylib.h"
#include "utils.h"

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define ROOM_WIDTH 8
#define ROOM_HEIGHT 8
#define ROOM_CELLS_LENGTH ROOM_WIDTH * ROOM_HEIGHT
#define WORLD_WIDTH 10
#define WORLD_HEIGHT 5
#define GAME_AREA_WIDTH TILE_WIDTH * ROOM_WIDTH
#define GAME_AREA_HEIGHT TILE_HEIGHT * ROOM_HEIGHT
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
    int cells[ROOM_CELLS_LENGTH];
    int x;
    int y;
    int width;
} Grid;

typedef struct Player
{
    Rectangle rect;
    Vector2 velocity;
    Vector2 movementRemainder;
} Player;

typedef struct GameState
{
    Grid currentRoom;
    Player player;
} GameState;

typedef struct EditorState
{
    int cursorX;
    int cursorY;
    Texture2D selector;
    bool active;
} EditorState;

/* ----------------------- Local Function Declaration ----------------------- */
const Viewport ViewportInit(int width, int height, int scale);

const int RecGetCenterX(Rectangle rec){return rec.x + rec.width / 2;}
const int RecGetCenterY(Rectangle rec){return rec.y + rec.height / 2;}

static void DrawViewport(Viewport viewport, GameState gamestate, Texture2D tex);
static void DrawEditorUI(EditorState editorState);
static void DrawWorld(Viewport vp, GameState state, Texture2D tex);

static void RoomSave(const Grid *grid);
static void RoomLoad(Grid *room);
const int RoomGetWidth(){return TILE_WIDTH * ROOM_WIDTH;}
const int RoomGetHeight(){return TILE_HEIGHT * ROOM_HEIGHT;}

const int GridGet(const Grid *grid, int x, int y);
const int GridGetHeight(Grid grid);
const bool CheckCollisionGridRec(const Grid *grid, Rectangle rect);
const bool CheckCollisionGridPoint(const Grid *grid, int x, int y);
static void GridSet(Grid *grid, int value, int x, int y);

static void PlayerMoveX(GameState *gamestate, float amount);
static void PlayerMoveY(GameState *gamestate, float amount);

int main()
{
    /* ----------------------------- Initialization ----------------------------- */
    int windowWidth = TILE_WIDTH * ROOM_WIDTH * PIXEL_SIZE;
    int windowHeight = 0;

    /* ------------------------ Window Size and Position ------------------------ */
    InitWindow(100, 100, "Game");
    windowWidth = GAME_AREA_WIDTH * PIXEL_SIZE * GetWindowScaleDPI().x;
    windowHeight = GAME_AREA_HEIGHT * PIXEL_SIZE * GetWindowScaleDPI().y;
    SetWindowSize(windowWidth, windowHeight);
    SetWindowPosition(GetMonitorWidth(0) / 2 - windowWidth / 2, GetMonitorHeight(0) / 2 - windowHeight / 2);

    Viewport viewport = ViewportInit(GAME_AREA_WIDTH, GAME_AREA_HEIGHT, 3 * GetWindowScaleDPI().x);

    Camera2D worldSpaceCamera = { 0 };  // Game world camera
    Camera2D screenSpaceCamera = { 0 }; // Smoothing camera
    worldSpaceCamera.zoom = 1.0f;
    screenSpaceCamera.zoom = 1.0f;

    /* ---------------------------- Loading Textures ---------------------------- */
    Texture2D tileset = LoadTexture("data/texture_tileset_01.png");
    Texture2D selector = LoadTexture("data/texture_ui_selector.png");

    /* ----------------------------- Init Game State ---------------------------- */
    GameState gamestate = {0};
    gamestate.currentRoom.width = ROOM_WIDTH;
    for (int i = 0; i < ROOM_CELLS_LENGTH; i++)
    {
        gamestate.currentRoom.cells[i] = 1;
    }
    RoomLoad(&gamestate.currentRoom);
    gamestate.player.rect.x = gamestate.player.rect.y = TILE_WIDTH+2;
    gamestate.player.rect.width = 14;
    gamestate.player.rect.height = 26;
    gamestate.player.velocity.x = gamestate.player.velocity.y = 0.0f;
    gamestate.currentRoom.x = gamestate.currentRoom.y = 0;

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
            if(IsKeyPressed(KEY_SPACE))
            {   
                int value = !GridGet(&gamestate.currentRoom, editorState.cursorX, editorState.cursorY);
                GridSet(&gamestate.currentRoom, value, editorState.cursorX, editorState.cursorY);
            }
            if(IsKeyDown(KEY_LEFT_CONTROL))
            {
                if(IsKeyPressed(KEY_S))
                {
                    RoomSave(&gamestate.currentRoom);
                }
                gamestate.player.rect.x += (IsKeyPressed(KEY_RIGHT) + -IsKeyPressed(KEY_LEFT)) * RoomGetWidth();
                gamestate.player.rect.y += (IsKeyPressed(KEY_DOWN) + -IsKeyPressed(KEY_UP)) * RoomGetHeight();
            }
            else
            {
                editorState.cursorX += IsKeyPressed(KEY_RIGHT) + -IsKeyPressed(KEY_LEFT);
                editorState.cursorY += IsKeyPressed(KEY_DOWN) + -IsKeyPressed(KEY_UP);
            }
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

        if(gamestate.currentRoom.x != floor(RecGetCenterX(gamestate.player.rect) / RoomGetWidth()))
        {
            gamestate.currentRoom.x = floor(RecGetCenterX(gamestate.player.rect) / RoomGetWidth());
            RoomLoad(&gamestate.currentRoom);
        }
        else if(gamestate.currentRoom.y != floor(gamestate.player.rect.y / RoomGetHeight()))
        {
            gamestate.currentRoom.y = floor(gamestate.player.rect.y / RoomGetHeight());
            RoomLoad(&gamestate.currentRoom);
        }
        worldSpaceCamera.target.x = gamestate.currentRoom.x * RoomGetWidth();

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
    for (int i = 0; i < ROOM_CELLS_LENGTH; i++)
    {
        Rectangle src = {state.currentRoom.cells[i] * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};
        Vector2 pos = {i % state.currentRoom.width * TILE_WIDTH + state.currentRoom.x * RoomGetWidth(), i / state.currentRoom.width * TILE_HEIGHT + state.currentRoom.y * RoomGetHeight()};
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
    return ROOM_CELLS_LENGTH / grid.width;
}

const bool CheckCollisionGridPoint(const Grid *grid, int x, int y)
{
    if(GridGet(grid, (x - grid->x * RoomGetWidth()) / TILE_WIDTH, (y - grid->y * RoomGetHeight()) / TILE_HEIGHT) > 0) return true;
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

static void RoomSave(const Grid *grid)
{
    int expectedDataSize = ROOM_WIDTH * ROOM_HEIGHT * WORLD_WIDTH * WORLD_HEIGHT * sizeof(int);
    int fileDataSize;
    unsigned char *data = 0;
    
    if(!FileExists("level.bin"))
    {
        int default_data[expectedDataSize];
        for(int i = 0; i < expectedDataSize; i++) { default_data[i] = 1;}
        SaveFileData("level.bin", &default_data, expectedDataSize);
        //SaveFileData("level.bin", (void*)grid->cells, sizeof(grid->cells));
        
    }
    
    data = LoadFileData("level.bin", &fileDataSize);

    if(fileDataSize != expectedDataSize)
    {
        UnloadFileData(data);
        int default_data[expectedDataSize];
        for(int i = 0; i < expectedDataSize; i++) { default_data[i] = 1;}
        SaveFileData("level.bin", &default_data, expectedDataSize);
        return;
    }
    int offset = (grid->y * WORLD_WIDTH + grid->x) * (ROOM_WIDTH * ROOM_HEIGHT) * sizeof(int);
    for(int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; i++)
    {
        unsigned char *ptr = data + offset + i * sizeof(int);
        *ptr = grid->cells[i];
    }
    SaveFileData("level.bin", data, expectedDataSize);
    UnloadFileData(data);
}

static void RoomLoad(Grid *room)
{
    int expectedDataSize = ROOM_WIDTH * ROOM_HEIGHT * WORLD_WIDTH * WORLD_HEIGHT * sizeof(int);
    int fileDataSize = 0;

    if(!FileExists("level.bin")) return;
    unsigned char *data = 0;
    data = LoadFileData("level.bin", &fileDataSize);
    

    // If data doesn't match the expected size, make default room and return;
    if(fileDataSize != expectedDataSize)
    {
        for (int i = 0; i < ROOM_CELLS_LENGTH; i++)
        {
            room->cells[i] = 1;
        }
        return;
    }


    // Data exists, load it
    int offset = (room->y * WORLD_WIDTH + room->x) * (ROOM_WIDTH * ROOM_HEIGHT) * sizeof(int);
    for(int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; i++)
    {
        unsigned char *ptr = data + offset + i * sizeof(int);
        room->cells[i] = *ptr;
    }

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
        if(CheckCollisionGridRec(&gamestate->currentRoom, rect))
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
        if(CheckCollisionGridRec(&gamestate->currentRoom, rect))
        {
            gamestate->player.velocity.y = 0;
            break;
        }
        gamestate->player.rect.y += dir;
        move -= dir;
    }

    return;
}