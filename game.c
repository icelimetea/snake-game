#include <stdlib.h>
#include <string.h>
#include "game.h"

// World

// Internal

int updateWorldPlayers(struct World* world, int start, int stop) {
	int emptySlot = start;

	for (int idx = start; idx < stop; idx++) {
		if (!isPlayerDead(world->players[idx])) {
			updatePlayer(world->players[idx]);
			world->players[emptySlot] = world->players[idx];
			emptySlot++;
		} else {
			freePlayer(world->players[idx]);
		}
	}

	return emptySlot - start;
}

bool addPlayer(struct World* world, struct Player* player) {
	if (world->numPlayers >= world->playersCapacity) {
		int oldCapacity = world->playersCapacity;
		int newCapacity = 2 * oldCapacity;

		struct Player** reallocated = realloc(world->players, newCapacity * sizeof(struct Player*));

		if (reallocated == NULL)
			return false;

		world->players = reallocated;
		world->playersCapacity = newCapacity;
	}

	world->players[world->numPlayers] = player;
	world->numPlayers++;

	return true;
}

// Public API

struct World* createWorld(int arenaWidth, int arenaHeight) {
	struct World* world = NULL;
	struct Player** players = NULL;
	struct TileArena* tileArena = NULL;

	world = malloc(sizeof(struct World));

	if (world == NULL)
		goto fail;

	players = malloc(INITIAL_PLAYER_CAPACITY * sizeof(struct Player*));

	if (players == NULL)
		goto fail;

	tileArena = createTileArena(arenaWidth, arenaHeight);

	if (tileArena == NULL)
		goto fail;

	world->numPlayers = 0;
	world->playersCapacity = INITIAL_PLAYER_CAPACITY;

	world->players = players;

	world->tileArena = tileArena;

	return world;
fail:
	free(world);
	freeTileArena(tileArena);
	free(players);
	return NULL;
}

void updateWorld(struct World* world) {
	world->numPlayers = updateWorldPlayers(world, 0, world->numPlayers);
}

struct TileArena* getWorldTileArena(struct World* world) {
	return world->tileArena;
}

void generateApple(struct World* world) {
	int x;
	int y;

	int attempts = 0;

	do {
		x = rand() % getTileArenaWidth(world->tileArena);
		y = rand() % getTileArenaHeight(world->tileArena);
		attempts++;
	} while (attempts < MAX_APPLE_GENERATION_ATTEMPTS && getTile(world->tileArena, x, y) != EMPTY_TILE);

	setTile(world->tileArena, x, y, APPLE_TILE);
}

void freeWorld(struct World* world) {
	if (world == NULL)
		return;

	for (int idx = 0; idx < world->numPlayers; idx++)
		freePlayer(world->players[idx]);

	free(world->players);
	freeTileArena(world->tileArena);
	free(world);
}

// Players

// Internal

bool appendPlayerSnakeHead(struct Player* player, struct SnakePart* part) {
	if (player->partsCount >= player->partsCapacity) {
		int oldCapacity = player->partsCapacity;
		int newCapacity = 2 * oldCapacity;

		struct SnakePart* reallocated = realloc(player->parts, newCapacity * sizeof(struct SnakePart));

		if (reallocated == NULL)
			return false;

		int tailElements = player->partsCount - player->headIndex - 1;

		memcpy(&reallocated[newCapacity - tailElements], &reallocated[oldCapacity - tailElements], tailElements * sizeof(struct SnakePart));

		player->parts = reallocated;
		player->partsCapacity = newCapacity;
	}

	struct TileArena* tileArena = getWorldTileArena(player->world);
	setTile(tileArena, part->x, part->y, SNAKE_TILE);

	player->headIndex = (player->headIndex + 1) % player->partsCapacity;
	player->parts[player->headIndex] = *part;

	player->partsCount++;

	return true;
}

void movePlayerSnakeHead(struct Player* player, struct SnakePart* part) {
	int currTail = (player->headIndex + player->partsCapacity - (player->partsCount - 1)) % player->partsCapacity;
	int nextHead = (player->headIndex + 1) % player->partsCapacity;

	struct TileArena* tileArena = getWorldTileArena(player->world);

	setTile(tileArena, player->parts[currTail].x, player->parts[currTail].y, EMPTY_TILE);
	setTile(tileArena, part->x, part->y, SNAKE_TILE);

	player->headIndex = nextHead;
	player->parts[player->headIndex] = *part;
}

int getPlayerSnakeLengthLimit(struct Player* player) {
	return player->score + 1;
}

// Public API

struct Player* createPlayer(struct World* world, Direction direction, int spawnX, int spawnY) {
	struct Player* player = NULL;
	struct SnakePart* parts = NULL;

	player = malloc(sizeof(struct Player));

	if (player == NULL)
		goto fail;

	parts = malloc(INITIAL_SNAKE_PARTS_CAPACITY * sizeof(struct SnakePart));

	if (parts == NULL)
		goto fail;

	player->world = world;

	player->direction = direction;

	player->dead = false;

	player->score = 0;

	player->partsCount = 1;
	player->partsCapacity = INITIAL_SNAKE_PARTS_CAPACITY;

	player->headIndex = 0;

	player->parts = parts;

	parts[0].x = spawnX;
	parts[0].y = spawnY;

	if (!addPlayer(world, player))
		goto fail;

	return player;
fail:
	free(player);
	free(parts);
	return NULL;
}

void setPlayerDirection(struct Player* player, Direction direction) {
	player->direction = direction;
}

void updatePlayer(struct Player* player) {
	if (player->dead)
		return;

	struct SnakePart currHead = player->parts[player->headIndex];
	struct SnakePart nextHead;

	switch (player->direction) {
	case UP:    nextHead.x = currHead.x; nextHead.y = currHead.y - 1; break;
	case DOWN:  nextHead.x = currHead.x; nextHead.y = currHead.y + 1; break;
	case LEFT:  nextHead.x = currHead.x - 1; nextHead.y = currHead.y; break;
	case RIGHT: nextHead.x = currHead.x + 1; nextHead.y = currHead.y; break;
	}

	struct TileArena* tileArena = getWorldTileArena(player->world);

	if (!isTilePointInBounds(tileArena, nextHead.x, nextHead.y)) {
		player->dead = true;
		return;
	}

	WorldTile tile = getTile(tileArena, nextHead.x, nextHead.y);

	if (tile == SNAKE_TILE) {
		player->dead = true;
		return;
	}

	if (tile == APPLE_TILE) {
		generateApple(player->world);
		player->score++;
	}

	if (player->partsCount < getPlayerSnakeLengthLimit(player)) {
		appendPlayerSnakeHead(player, &nextHead);
	} else {
		movePlayerSnakeHead(player, &nextHead);
	}
}

bool isPlayerDead(struct Player* player) {
	return player->dead;
}

int getPlayerScore(struct Player* player) {
	return player->score;
}

void freePlayer(struct Player* player) {
	if (player == NULL)
		return;

	free(player->parts);
	free(player);
}
