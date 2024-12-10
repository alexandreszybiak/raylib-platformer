#include "raylib.h"

/* ---------------------------------- Type ---------------------------------- */
typedef struct Viewport
{
    RenderTexture2D renderTexture2D;
    Rectangle rectSource;
    Rectangle rectDest;
} Viewport;

/* ----------------------- Local Variables Definition ----------------------- */

Rectangle rect = { 0, 0, 32, 32};

/* ----------------------- Local Function Declaration ----------------------- */

static Viewport ViewportInit(int width, int height, int scale);
static void DrawFrame(Viewport viewport);        // Update and draw one frame

int main()
{
    /* ----------------------------- Initialization ----------------------------- */
    const int gameAreaWidth = 160;
    const int gameAreaHeight = 240;
    int windowWidth = 0;
    int windowHeight = 0;

    /* ------------------------ Window Size and Position ------------------------ */
    InitWindow(100, 100, "Game");
    windowWidth = gameAreaWidth * 3 * GetWindowScaleDPI().x;
    windowHeight = gameAreaHeight * 3 * GetWindowScaleDPI().y;
    SetWindowSize(windowWidth, windowHeight);
    SetWindowPosition(GetMonitorWidth(0) / 2 - windowWidth / 2, GetMonitorHeight(0) / 2 - windowHeight / 2);

    Viewport viewport = ViewportInit(gameAreaWidth, gameAreaHeight, 3 * GetWindowScaleDPI().x);

    rect.x = rect.y = 0;

    SetTargetFPS(60);

    /* -------------------------------- Main Loop ------------------------------- */
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        DrawFrame(viewport);
    }

    /* ---------------------------- De-Initialization --------------------------- */
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}

// Update and draw game frame
static void DrawFrame(Viewport viewport)
{

    /* ---------------------------- Draw in Viewport ---------------------------- */
    BeginTextureMode(viewport.renderTexture2D);

    ClearBackground(DARKGREEN);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, RAYWHITE);
    DrawCircle(0, 0, 32, GREEN);

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