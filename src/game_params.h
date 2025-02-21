#ifndef GAME_PARAMS_H
#define GAME_PARAMS_H

#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define ROOM_WIDTH 20
#define ROOM_HEIGHT 16
#define ROOM_CELLS_LENGTH ROOM_WIDTH * ROOM_HEIGHT
#define WORLD_WIDTH 10
#define WORLD_HEIGHT 5
#define GAME_AREA_WIDTH TILE_WIDTH * ROOM_WIDTH
#define GAME_AREA_HEIGHT TILE_HEIGHT * ROOM_HEIGHT
#define PIXEL_SIZE 3

#define TILE_EMPTY 0
#define TILE_SAVE 1
#define TILE_WALL 2

#define FILENAME_WORLD "data/world.bin"
#define FILENAME_SAVE_1 "save1.bin"
#define FILENAME_SAVE_2 "save2.bin"
#define FILENAME_SAVE_3 "save3.bin"

#define NUM_SAVES 3

#endif