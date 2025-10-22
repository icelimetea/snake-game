#ifndef _GAME_H
#define _GAME_H

#include <stdbool.h>
#include "tile.h"

// World

// Player forward declaration

struct Player;

#define INITIAL_PLAYER_CAPACITY 4

#define MAX_APPLE_GENERATION_ATTEMPTS 5

struct World {
	int playersCount;
	int playersCapacity;

	struct Player** players;

	struct TileArena* tileArena;
};

struct World* createWorld(int arenaWidth, int arenaHeight);

void updateWorld(struct World* world);

struct TileArena* getWorldTileArena(struct World* world);

void generateApple(struct World* world);

void freeWorld(struct World* world);

// Player

#define INITIAL_SNAKE_PARTS_CAPACITY 4

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
	struct World* world;

	struct {
		Direction direction;
		bool dead;
		int score;
		int refcount;
	} properties;

	int partsCount;
	int partsCapacity;

	int headIndex;

	struct SnakePart* parts;
};

struct Player* createPlayer(struct World* world, Direction direction, int spawnX, int spawnY);

void updatePlayer(struct Player* player);

void setPlayerDirection(struct Player* player, Direction direction);
Direction getPlayerDirection(struct Player* player);

void markPlayerAsDead(struct Player* player);
bool isPlayerDead(struct Player* player);

void incrementPlayerScore(struct Player* player);
int getPlayerScore(struct Player* player);

void leakPlayer(struct Player* player);
void freePlayer(struct Player* player);

#endif
