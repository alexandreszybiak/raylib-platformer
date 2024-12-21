#include <math.h>
#include "raylib.h"
#include "utils.h"
#include "game_params.h"
#include "grid.h"

/* ---------------------------------- Type ---------------------------------- */
typedef struct Viewport
{
    RenderTexture2D renderTexture2D;
    Rectangle rectSource;
    Rectangle rectDest;
} Viewport;

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

typedef struct Command
{
    unsigned int epoch;
    unsigned int lifetime;
} Command;

typedef struct CommandState
{
    int move;
    unsigned char validate;
    unsigned char save;
    Command jump;
} CommandState;

typedef struct EditorCommandState
{
    unsigned char save;
    unsigned char load;
    unsigned char edit;
    char tileValue;
    unsigned char toggle;
    int moveX;
    int moveY;
    int moveCursorX;
    int moveCursorY;
} EditorCommandState;

typedef enum GameScreen { GAMESCREEN_TITLE = 0, GAMESCREEN_LOAD, GAMESCREEN_PLAY } GameScreen;

/* ----------------------- Local Function Declaration ----------------------- */
const Viewport ViewportInit(int width, int height, int scale);

const int RecGetCenterX(Rectangle rec){return rec.x + rec.width / 2;}
const int RecGetCenterY(Rectangle rec){return rec.y + rec.height / 2;}

static void ProcessInputs();
static void Update();

static void Draw();
static void DrawViewport();
static void DrawEditorUI();
static void DrawWorld();

static void PlayerMoveX(float amount);
static void PlayerMoveY(float amount);

void GameSave();
void GameLoad();

void GameStateUpdateCurrentRoom(GameState *gameState);

/* ------------------------------- Init Memory ------------------------------ */
Viewport viewport = {0};
Texture2D tex_tileset = {0};
Texture2D tex_selector = {0};
Camera2D worldSpaceCamera = { 0 };  // Game world camera
Camera2D screenSpaceCamera = { 0 }; // Smoothing camera
GameState gameState = {0};
EditorState editorState = {0};
EditorCommandState editorCommands = {0};
EditorCommandState editorCommandsEmpty = {0};
CommandState commandState = {0};
CommandState commandStateEmpty = {0};
GameScreen gameScreen = GAMESCREEN_TITLE;


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

    viewport = ViewportInit(GAME_AREA_WIDTH, GAME_AREA_HEIGHT, 3 * GetWindowScaleDPI().x);
    worldSpaceCamera.zoom = 1.0f;
    screenSpaceCamera.zoom = 1.0f;

    /* ---------------------------- Loading Textures ---------------------------- */
    tex_tileset = LoadTexture("data/texture_tileset_01.png");
    tex_selector = LoadTexture("data/texture_ui_selector.png");

    /* ----------------------------- Init Game State ---------------------------- */
    gameState.currentRoom.width = ROOM_WIDTH;
    RoomLoad(&gameState.currentRoom);
    gameState.player.rect.x = gameState.player.rect.y = TILE_WIDTH+2;
    gameState.player.rect.width = 14;
    gameState.player.rect.height = 26;
    gameState.player.velocity.x = gameState.player.velocity.y = 0.0f;
    gameState.currentRoom.x = gameState.currentRoom.y = 0;
    GameLoad();
    GameStateUpdateCurrentRoom(&gameState);

    /* ---------------------------- Init Editor State --------------------------- */
    editorState.cursorX = editorState.cursorY = 0;
    editorState.selector = tex_selector;
    editorState.active = false;

    /* -------------------------------- Main Loop ------------------------------- */
    SetTargetFPS(60);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        ProcessInputs();
        Update();
        Draw();
    }

    /* ---------------------------- De-Initialization --------------------------- */
    UnloadRenderTexture(viewport.renderTexture2D);
    UnloadTexture(tex_selector);
    UnloadTexture(tex_tileset);
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}

static void ProcessInputs()
{
    editorCommands = editorCommandsEmpty;
    commandState = commandStateEmpty;

    if(gameScreen == GAMESCREEN_TITLE)
    {
        if(IsKeyPressed(KEY_SPACE)) 
            commandState.validate = true;
        return;
    }
        
    if(editorState.active == true)
    {
        if(IsKeyDown(KEY_LEFT_CONTROL))
        {
            if(IsKeyPressed(KEY_S)) editorCommands.save = true;
            editorCommands.moveX = (IsKeyPressed(KEY_RIGHT) + -IsKeyPressed(KEY_LEFT));
            editorCommands.moveY = (IsKeyPressed(KEY_DOWN) + -IsKeyPressed(KEY_UP));
            return;
        }
        editorCommands.moveCursorX = IsKeyPressed(KEY_RIGHT) + -IsKeyPressed(KEY_LEFT);
        editorCommands.moveCursorY = IsKeyPressed(KEY_DOWN) + -IsKeyPressed(KEY_UP);
        if(IsKeyPressed(KEY_SPACE)) editorCommands.tileValue = TILE_EMPTY;
        if(IsKeyPressed(KEY_S)) editorCommands.tileValue = TILE_SAVE;
        
        return;
    }

    if(IsKeyPressed(KEY_P)) editorCommands.toggle = true;

    if(IsKeyPressed(KEY_BACKSPACE)) gameScreen = GAMESCREEN_TITLE;
    if(IsKeyPressed(KEY_DOWN)) commandState.save = true;

    commandState.move = IsKeyDown(KEY_RIGHT) + -IsKeyDown(KEY_LEFT);
    if(IsKeyPressed(KEY_UP)) commandState.jump.epoch = 1;
}

static void Update()
{
    /* --------------------------- Title Screen Update -------------------------- */
    if(gameScreen == GAMESCREEN_TITLE)
    {
        if(commandState.validate)
            gameScreen = GAMESCREEN_PLAY;
        return;
    }

    /* ------------------------------ Editor Update ----------------------------- */
    if(editorState.active)
    {
        if(editorCommands.toggle)
        {
            editorState.active = false;
            return;
        }
        if(editorCommands.save) RoomSave(&gameState.currentRoom);
        gameState.player.rect.x += editorCommands.moveX * RoomGetWidth();
        gameState.player.rect.y += editorCommands.moveY * RoomGetHeight();
        editorState.cursorX += editorCommands.moveCursorX;
        editorState.cursorY += editorCommands.moveCursorY;
        if(editorCommands.tileValue > -1)
        {
            GridSet(&gameState.currentRoom, editorCommands.tileValue, editorState.cursorX, editorState.cursorY);
        }
        
        return;
    }

    /* ---------------------------- Game State Update --------------------------- */
    if(commandState.save && CheckCollisionGridTileRec(&gameState.currentRoom, TILE_SAVE, gameState.player.rect)) GameSave(gameState);

    Rectangle rec = gameState.player.rect;
    rec.y += 2;
    if(CheckCollisionGridTileRec(&gameState.currentRoom, TILE_WALL, rec)){
        gameState.player.velocity.y += commandState.jump.epoch * -4.0f;
    }

    gameState.player.velocity.x = commandState.move * 2;
    gameState.player.velocity.y += 0.2f;
    PlayerMoveX(gameState.player.velocity.x);
    PlayerMoveY(gameState.player.velocity.y);
    if(IsKeyPressed(KEY_P)) editorState.active = !editorState.active;
    GameStateUpdateCurrentRoom(&gameState);

    worldSpaceCamera.target.x = gameState.currentRoom.x * RoomGetWidth();
}

static void Draw()
{
    /* -------------------------------- Viewport -------------------------------- */
    BeginTextureMode(viewport.renderTexture2D);
    BeginMode2D(worldSpaceCamera);

    if(gameScreen == GAMESCREEN_TITLE)
    {
        ClearBackground(GRAY);
        
    }
    else
    {
        DrawWorld();
        if(editorState.active) DrawEditorUI();

    }

    EndMode2D();
    EndTextureMode();

    /* --------------------------------- Window --------------------------------- */
    BeginDrawing();
    BeginMode2D(screenSpaceCamera);
    DrawViewport();
    DrawFPS(GetScreenWidth() - 95, 10);
    EndMode2D();
    EndDrawing();
}

// Update and draw game frame
static void DrawViewport()
{
    /* ------------------------- Draw Viewport in Window ------------------------ */
    ClearBackground(BLACK);

    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(viewport.renderTexture2D.texture, viewport.rectSource, viewport.rectDest, origin, 0.0f, WHITE);
}

static void DrawWorld()
{    
    ClearBackground(DARKGREEN);

    /* ---------------------------------- Grid ---------------------------------- */
    for (int i = 0; i < ROOM_CELLS_LENGTH; i++)
    {
        Rectangle src = {gameState.currentRoom.cells[i] * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};
        Vector2 pos = {i % gameState.currentRoom.width * TILE_WIDTH + gameState.currentRoom.x * RoomGetWidth(), i / gameState.currentRoom.width * TILE_HEIGHT + gameState.currentRoom.y * RoomGetHeight()};
        DrawTextureRec(tex_tileset, src, pos, WHITE);
    }

    /* ------------------------------- Draw Player ------------------------------ */
    DrawRectangleRec(gameState.player.rect, WHITE);
}

static void DrawEditorUI()
{   
    DrawTexture(tex_selector, editorState.cursorX * TILE_WIDTH, editorState.cursorY * TILE_HEIGHT, GREEN);
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

void GameStateUpdateCurrentRoom(GameState *gameState)
{
    if(gameState->currentRoom.x != floor(RecGetCenterX(gameState->player.rect) / RoomGetWidth()))
    {
        gameState->currentRoom.x = floor(RecGetCenterX(gameState->player.rect) / RoomGetWidth());
        RoomLoad(&gameState->currentRoom);
    }
    else if(gameState->currentRoom.y != floor(gameState->player.rect.y / RoomGetHeight()))
    {
        gameState->currentRoom.y = floor(gameState->player.rect.y / RoomGetHeight());
        RoomLoad(&gameState->currentRoom);
    }
}

void GameSave()
{
    int data[2];
    data[0] = gameState.player.rect.x;
    data[1] = gameState.player.rect.y;
    SaveFileData(FILENAME_SAVE, &data, sizeof(data));
}

void GameLoad()
{
    int dataSize = 0;

    void *data = LoadFileData(FILENAME_SAVE, &dataSize);
    if(dataSize != sizeof(int) * 2) return;

    int *ptr = data;
    gameState.player.rect.x = *ptr;
    gameState.player.rect.y = *(ptr + 1);
    
}

static void PlayerMoveX(float amount)
{
    gameState.player.movementRemainder.x += amount;
    int move = round(gameState.player.movementRemainder.x);
    if(move == 0) return;
    gameState.player.movementRemainder.x -= move;
    int dir = signf(move);
    while(move != 0)
    {
        Rectangle rect = gameState.player.rect;
        rect.x += dir;
        if(CheckCollisionGridTileRec(&gameState.currentRoom, TILE_WALL, rect))
        {
            gameState.player.velocity.x = 0;
            break;
        }
        gameState.player.rect.x += dir;
        move -= dir;
    }

    return;
}

static void PlayerMoveY(float amount)
{
    gameState.player.movementRemainder.y += amount;
    int move = round(gameState.player.movementRemainder.y);
    if(move == 0) return;
    gameState.player.movementRemainder.y -= move;
    int dir = signf(move);
    while(move != 0)
    {
        Rectangle rect = gameState.player.rect;
        rect.y += dir;
        if(CheckCollisionGridTileRec(&gameState.currentRoom, TILE_WALL, rect))
        {
            gameState.player.velocity.y = 0;
            break;
        }
        gameState.player.rect.y += dir;
        move -= dir;
    }

    return;
}