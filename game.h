#ifndef _GAME_H
#define _GAME_H

#include <stdbool.h>
#include <stdatomic.h>
#include "tile.h"

// World

// Player forward declaration

struct Player;

#define MAX_APPLE_GENERATION_ATTEMPTS 5

struct World {
	_Atomic(struct Player*) players;
	struct TileArena* tileArena;
};

struct World* createWorld(int arenaWidth, int arenaHeight);

void updateWorld(struct World* world);

void generateApple(struct World* world);

struct TileArena* getWorldTileArena(struct World* world);

void freeWorld(struct World* world);

// Player

// Must be a power of two
#define INITIAL_SNAKE_PARTS_CAPACITY (1 << 2)

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT
} Direction;

struct SnakePart {
	int x;
	int y;
};

struct Player {
	struct Player* next;

	struct World* world;

	int partsCount;
	int partsCapacity;
	int headIndex;
	struct SnakePart* parts;

	atomic_int refcount;
	_Atomic(Direction) direction;
	atomic_int score;
	atomic_bool dead;
};

struct Player* createPlayer(struct World* world, Direction direction, int spawnX, int spawnY);

bool updatePlayer(struct Player* player);

void setPlayerDirection(struct Player* player, Direction direction);
Direction getPlayerDirection(struct Player* player);

void incrementPlayerScore(struct Player* player);
int getPlayerScore(struct Player* player);

void markPlayerAsDead(struct Player* player);
bool isPlayerDead(struct Player* player);

void leakPlayer(struct Player* player);
void freePlayer(struct Player* player);

#endif
