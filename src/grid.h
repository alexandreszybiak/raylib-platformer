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
const int GridGet(const Grid *grid, int x, int y);
const int GridGetHeight(Grid grid);
const bool CheckCollisionGridRec(const Grid *grid, Rectangle rect);
const bool CheckCollisionGridPoint(const Grid *grid, int x, int y);

#endif