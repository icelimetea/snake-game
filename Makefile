CFLAGS+=-Wall -Wextra -Wpedantic -Werror -std=gnu11

ifeq (${BUILD},Release)
	CFLAGS+=-O3 -flto
else
	CFLAGS+=-O2 -g
endif

OBJS=main.o game.o tile.o

snake: ${OBJS}
	${CC} ${CFLAGS} -o snake ${OBJS}

main.o: main.c game.h tile.h
game.o: game.c game.h tile.h
tile.o: tile.c tile.h

.PHONY: clean
clean:
	rm -f snake ${OBJS}
