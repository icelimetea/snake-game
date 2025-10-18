#ifndef _TILE_H
#define _TILE_H

#include <stdbool.h>

typedef enum {
	EMPTY_TILE,
	SNAKE_TILE,
	APPLE_TILE
} WorldTile;

struct TileArena {
	int arenaWidth;
	int arenaHeight;

	WorldTile tiles[];
};

struct TileArena* createTileArena(int arenaWidth, int arenaHeight);

bool isTilePointInBounds(struct TileArena* tileArena, int x, int y);

int getTileArenaWidth(struct TileArena* tileArena);
int getTileArenaHeight(struct TileArena* tileArena);

WorldTile getTile(struct TileArena* tileArena, int x, int y);
void setTile(struct TileArena* tileArena, int x, int y, WorldTile tile);

void freeTileArena(struct TileArena* tileArena);

#endif
