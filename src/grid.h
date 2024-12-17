#ifndef GRID_H
#define GRID_H

#include "raylib.h"
#include "game_params.h"

typedef struct Grid
{
    int cells[ROOM_CELLS_LENGTH];
    int x;
    int y;
    int width;
} Grid;

void RoomSave(const Grid *grid);
void RoomLoad(Grid *room);
const int RoomGetWidth();
const int RoomGetHeight();

void GridSet(Grid *grid, int value, int x, int y);
void GridFill(Grid *grid, int value);
const int GridGet(const Grid *grid, int x, int y);
const int GridGetHeight(Grid grid);
const bool CheckCollisionGridTilePoint(const Grid *grid, int tile, int x, int y);
const bool CheckCollisionGridTileRec(const Grid *grid, int tile, Rectangle rect);

#endif