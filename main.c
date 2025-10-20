#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include "game.h"

#define ARENA_WIDTH 20
#define ARENA_HEIGHT 20

#define FRAME_DELAY_MS 100

void render(struct TileArena* tileArena) {
	puts("\x1B[2J\x1B[1;1H");

	for (int y = 0; y < getTileArenaHeight(tileArena); y++) {
		for (int x = 0; x < getTileArenaWidth(tileArena); x++) {
			switch (getTile(tileArena, x, y)) {
			case EMPTY_TILE: putchar('.'); break;
			case SNAKE_TILE: putchar('#'); break;
			case APPLE_TILE: putchar('@'); break;
			}
		}

		putchar('\n');
	}
}

int main(void) {
	srand(time(NULL));

	struct World* world = createWorld(ARENA_WIDTH, ARENA_HEIGHT);

	if (world == NULL)
		return 1;

	struct Player* player = createPlayer(world, UP, ARENA_WIDTH / 2, ARENA_HEIGHT / 2);

	if (player == NULL) {
		freeWorld(world);
		return 2;
	}

	generateApple(world);

	struct termios termConf;
	tcgetattr(STDOUT_FILENO, &termConf);

	struct termios origConf = termConf;

	termConf.c_lflag &= ~(ECHO | ICANON);
	termConf.c_cc[VMIN] = 0;
	termConf.c_cc[VTIME] = 0;
	tcsetattr(STDOUT_FILENO, 0, &termConf);

	char ch;

	while (!isPlayerDead(player)) {
		if (read(STDIN_FILENO, &ch, 1)) {
			switch (ch) {
			case 'w': setPlayerDirection(player, UP); break;
			case 's': setPlayerDirection(player, DOWN); break;
			case 'a': setPlayerDirection(player, LEFT); break;
			case 'd': setPlayerDirection(player, RIGHT); break;
			}
		}

		updateWorld(world);

		render(getWorldTileArena(world));
		printf("Score: %d\n", getPlayerScore(player));

		usleep(1000 * FRAME_DELAY_MS);
	}

	tcsetattr(STDOUT_FILENO, 0, &origConf);

	freeWorld(world);
	freePlayer(player);
	return 0;
}
