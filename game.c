#include <stdlib.h>
#include <string.h>
#include "game.h"

// World

struct World* createWorld(int arenaWidth, int arenaHeight) {
	struct World* world = NULL;
	struct TileArena* tileArena = NULL;

	world = malloc(sizeof(struct World));

	if (world == NULL)
		goto fail;

	tileArena = createTileArena(arenaWidth, arenaHeight);

	if (tileArena == NULL)
		goto fail;

	world->players = NULL;
	world->tileArena = tileArena;

	return world;
fail:
	free(world);
	freeTileArena(tileArena);
	return NULL;
}

void addPlayer(struct World* world, struct Player* player) {
	player->next = world->players;
	world->players = player;
	leakPlayer(player);
}

void updateWorld(struct World* world) {
	struct Player* player = world->players;

	while (player != NULL && !updatePlayer(player)) {
		world->players = player->next;
		freePlayer(player);

		player = world->players;
	}

	if (player == NULL)
		return;

	while (player->next != NULL) {
		if (!updatePlayer(player->next)) {
			struct Player* pivot = player->next;
			player->next = pivot->next;
			freePlayer(pivot);
		} else {
			player = player->next;
		}
	}
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

	struct Player* player = world->players;

	while (player != NULL) {
		struct Player* next = player->next;
		freePlayer(player);
		player = next;
	}

	freeTileArena(world->tileArena);
	free(world);
}

// Players

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

	player->refcount = 1;

	setPlayerDirection(player, direction);
	player->properties.dead = false;
	player->properties.score = 0;

	player->partsCount = 1;
	player->partsCapacity = INITIAL_SNAKE_PARTS_CAPACITY;

	player->headIndex = 0;

	player->parts = parts;

	parts[0].x = spawnX;
	parts[0].y = spawnY;

	addPlayer(world, player);

	return player;
fail:
	free(player);
	free(parts);
	return NULL;
}

int getPlayerSnakePartIndex(struct Player* player, int offset) {
	return (player->headIndex + offset) & (player->partsCapacity - 1);
}

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

	player->headIndex = getPlayerSnakePartIndex(player, 1);
	player->parts[player->headIndex] = *part;

	player->partsCount++;

	return true;
}

struct SnakePart movePlayerSnakeHead(struct Player* player, struct SnakePart* part) {
	struct SnakePart evicted = player->parts[getPlayerSnakePartIndex(player, -(player->partsCount - 1))];

	player->headIndex = getPlayerSnakePartIndex(player, 1);
	player->parts[player->headIndex] = *part;

	return evicted;
}

bool updatePlayer(struct Player* player) {
	struct PlayerProperties props = getPlayerProperties(player);

	if (props.dead)
		return false;

	struct SnakePart currHead = player->parts[player->headIndex];
	struct SnakePart nextHead;

	switch (props.direction) {
	case UP:    nextHead.x = currHead.x; nextHead.y = currHead.y - 1; break;
	case DOWN:  nextHead.x = currHead.x; nextHead.y = currHead.y + 1; break;
	case LEFT:  nextHead.x = currHead.x - 1; nextHead.y = currHead.y; break;
	case RIGHT: nextHead.x = currHead.x + 1; nextHead.y = currHead.y; break;
	}

	struct TileArena* tileArena = getWorldTileArena(player->world);

	if (!isTilePointInBounds(tileArena, nextHead.x, nextHead.y)) {
		markPlayerAsDead(player);
		return false;
	}

	WorldTile tile = getTile(tileArena, nextHead.x, nextHead.y);

	if (tile == SNAKE_TILE) {
		markPlayerAsDead(player);
		return false;
	}

	if (tile == APPLE_TILE) {
		generateApple(player->world);
		incrementPlayerScore(player);
	}

	if (player->partsCount < props.score + 1) {
		appendPlayerSnakeHead(player, &nextHead);
	} else {
		struct SnakePart prevTail = movePlayerSnakeHead(player, &nextHead);
		setTile(tileArena, prevTail.x, prevTail.y, EMPTY_TILE);
	}

	setTile(tileArena, nextHead.x, nextHead.y, SNAKE_TILE);

	return true;
}

void setPlayerDirection(struct Player* player, Direction direction) {
	player->properties.direction = direction;
}

void markPlayerAsDead(struct Player* player) {
	player->properties.dead = true;
}

void incrementPlayerScore(struct Player* player) {
	player->properties.score++;
}

struct PlayerProperties getPlayerProperties(struct Player* player) {
	return player->properties;
}

void leakPlayer(struct Player* player) {
	player->refcount++;
}

void freePlayer(struct Player* player) {
	if (player == NULL)
		return;

	player->refcount--;

	if (player->refcount == 0) {
		free(player->parts);
		free(player);
	}
}
