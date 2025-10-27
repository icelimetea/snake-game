#include <stdlib.h>
#include <string.h>
#include "game.h"

// Forward declaration of freePlayer function

void freePlayer(struct Player* player);

// World

struct World* createWorld(int arenaWidth, int arenaHeight) {
	struct World* world = NULL;
	struct TileArena* tileArena = NULL;

	world = aligned_alloc(CACHE_LINE_SIZE, sizeof(struct World));

	if (world == NULL)
		goto fail;

	tileArena = createTileArena(arenaWidth, arenaHeight);

	if (tileArena == NULL)
		goto fail;

	atomic_init(&world->players, NULL);

	world->tileArena = tileArena;

	return world;
fail:
	free(world);
	freeTileArena(tileArena);
	return NULL;
}

void addPlayer(struct World* world, struct Player* player) {
	player->next = atomic_load_explicit(&world->players, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&world->players, &player->next, player, memory_order_release, memory_order_relaxed))
		; // CAS happens in the loop conditional, no body is needed
}

void updateWorld(struct World* world) {
	struct Player* head = atomic_load_explicit(&world->players, memory_order_acquire);

	if (head == NULL)
		return;

	struct Player* player = head;

	while (player->next != NULL) {
		if (!updatePlayer(player->next)) {
			struct Player* pivot = player->next;
			player->next = pivot->next;
			freePlayer(pivot);
		} else {
			player = player->next;
		}
	}

	if (!updatePlayer(head) && atomic_compare_exchange_weak_explicit(&world->players, &head, head->next, memory_order_relaxed, memory_order_relaxed))
		freePlayer(head);
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

struct TileArena* getWorldTileArena(struct World* world) {
	return world->tileArena;
}

void freeWorld(struct World* world) {
	if (world == NULL)
		return;

	struct Player* player = atomic_load_explicit(&world->players, memory_order_acquire);

	while (player != NULL) {
		struct Player* next = player->next;
		freePlayer(player);
		player = next;
	}

	freeTileArena(world->tileArena);
	free(world);
}

// Players

// Thread-safe API

struct Player* createPlayer(struct World* world, int sockfd, Direction direction, int spawnX, int spawnY) {
	struct Player* player = NULL;
	struct SnakePart* parts = NULL;

	player = aligned_alloc(CACHE_LINE_SIZE, sizeof(struct Player));

	if (player == NULL)
		goto fail;

	parts = malloc(INITIAL_SNAKE_PARTS_CAPACITY * sizeof(struct SnakePart));

	if (parts == NULL)
		goto fail;

	player->world = world;

	player->sockfd = sockfd;

	atomic_init(&player->direction, direction);
	atomic_init(&player->dead, false);

	player->score = 0;

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

void setPlayerDirection(struct Player* player, Direction direction) {
	atomic_store_explicit(&player->direction, direction, memory_order_relaxed);
}

Direction getPlayerDirection(struct Player* player) {
	return atomic_load_explicit(&player->direction, memory_order_relaxed);
}

void markPlayerAsDead(struct Player* player) {
	atomic_store_explicit(&player->dead, true, memory_order_relaxed);
}

bool isPlayerDead(struct Player* player) {
	return atomic_load_explicit(&player->dead, memory_order_relaxed);
}

// END: Thread-safe API

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
	if (isPlayerDead(player))
		return false;

	struct SnakePart currHead = player->parts[player->headIndex];
	struct SnakePart nextHead;

	switch (getPlayerDirection(player)) {
	case UP:    nextHead.x = currHead.x; nextHead.y = currHead.y - 1; break;
	case DOWN:  nextHead.x = currHead.x; nextHead.y = currHead.y + 1; break;
	case LEFT:  nextHead.x = currHead.x - 1; nextHead.y = currHead.y; break;
	case RIGHT: nextHead.x = currHead.x + 1; nextHead.y = currHead.y; break;
	}

	struct TileArena* tileArena = getWorldTileArena(player->world);

	if (!isTilePointInBounds(tileArena, nextHead.x, nextHead.y)) {
		markPlayerAsDead(player);
		return true;
	}

	WorldTile tile = getTile(tileArena, nextHead.x, nextHead.y);

	if (tile == SNAKE_TILE) {
		markPlayerAsDead(player);
		return true;
	}

	if (tile == APPLE_TILE) {
		generateApple(player->world);
		incrementPlayerScore(player);
	}

	if (player->partsCount < getPlayerScore(player) + 1) {
		appendPlayerSnakeHead(player, &nextHead);
	} else {
		struct SnakePart prevTail = movePlayerSnakeHead(player, &nextHead);
		setTile(tileArena, prevTail.x, prevTail.y, EMPTY_TILE);
	}

	setTile(tileArena, nextHead.x, nextHead.y, SNAKE_TILE);

	return true;
}

void incrementPlayerScore(struct Player* player) {
	player->score++;
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
