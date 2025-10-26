#ifndef _GAME_H
#define _GAME_H

#include <stdbool.h>
#include <stdatomic.h>
#include "tile.h"

#define PADDING_SIZE 128
#define PADDING_FOR_OFFSET(x) (-(x) & (PADDING_SIZE - 1))

// World

// Player forward declaration

struct Player;

#define MAX_APPLE_GENERATION_ATTEMPTS 5

struct World {
	_Atomic(struct Player*) players;

	char __pad0[PADDING_FOR_OFFSET(8)];

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
	int __pad0;
	struct SnakePart* parts;

	char __pad1[PADDING_FOR_OFFSET(40)];

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
