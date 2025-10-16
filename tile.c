#include <stdlib.h>
#include <string.h>
#include "tile.h"

struct TileArena* createTileArena(int arenaWidth, int arenaHeight) {
	struct TileArena* tileArena = malloc(sizeof(struct TileArena) + arenaWidth * arenaHeight * sizeof(WorldTile));

	if (tileArena == NULL)
		return NULL;

	memset(tileArena->tiles, 0, arenaWidth * arenaHeight * sizeof(WorldTile));

	tileArena->arenaWidth = arenaWidth;
	tileArena->arenaHeight = arenaHeight;

	return tileArena;
}

int isTilePointInBounds(struct TileArena* tileArena, int x, int y) {
	return x >= 0 && x < tileArena->arenaWidth && y >= 0 && y < tileArena->arenaHeight;
}

int getTileArenaWidth(struct TileArena* tileArena) {
	return tileArena->arenaWidth;
}

int getTileArenaHeight(struct TileArena* tileArena) {
	return tileArena->arenaHeight;
}

WorldTile getTile(struct TileArena* tileArena, int x, int y) {
	if (!isTilePointInBounds(tileArena, x, y))
		return EMPTY_TILE;

	return tileArena->tiles[tileArena->arenaWidth * y + x];
}

void setTile(struct TileArena* tileArena, int x, int y, WorldTile tile) {
	if (!isTilePointInBounds(tileArena, x, y))
		return;

	tileArena->tiles[tileArena->arenaWidth * y + x] = tile;
}

void freeTileArena(struct TileArena* tileArena) {
	free(tileArena);
}
