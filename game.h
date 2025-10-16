#ifndef _GAME_H
#define _GAME_H

#include "tile.h"

// World

// Player forward declaration

struct Player;

#define INITIAL_PLAYER_CAPACITY 4
#define MAX_APPLE_GENERATION_ATTEMPTS 5

struct World {
	int numPlayers;
	int playersCapacity;

	struct Player** players;

	struct TileArena* tileArena;
};

struct World* createWorld(int arenaWidth, int arenaHeight);

int addPlayer(struct World* world, struct Player* player);

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

	Direction direction;

	int dead;

	int score;

	int partsCount;
	int partsCapacity;

	int headIndex;

	struct SnakePart* parts;
};

struct Player* createPlayer(struct World* world, Direction direction, int spawnX, int spawnY);

void setPlayerDirection(struct Player* player, Direction direction);

void updatePlayer(struct Player* player);

int isPlayerDead(struct Player* player);

int getPlayerScore(struct Player* player);

void freePlayer(struct Player* player);

#endif
