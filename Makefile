CFLAGS+=-Wall -Wextra -Wpedantic -Werror
CFLAGS+=-O3 -march=native -flto

OBJS=main.o game.o tile.o

snake: ${OBJS}
	${CC} ${CFLAGS} -o snake ${OBJS}

main.o: main.c game.h tile.h
game.o: game.c game.h tile.h
tile.o: tile.c tile.h

.PHONY: clean
clean:
	rm -f snake ${OBJS}
