#include "grid.h"

const int RoomGetWidth()
{
    return TILE_WIDTH * ROOM_WIDTH;
}
const int RoomGetHeight()
{
    return TILE_HEIGHT * ROOM_HEIGHT;
}

const int GridGet(const Grid *grid, int x, int y)
{
    if(x < 0 || x >= grid->width) return 0;
    if(y < 0 || y >= GridGetHeight(*grid)) return 0;
    return grid->cells[y * grid->width + x];
}

void GridSet(Grid *grid, int value, int x, int y)
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

void RoomSave(const Grid *grid)
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

void RoomLoad(Grid *room)
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