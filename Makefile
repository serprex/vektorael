all:vek vez
CC=gcc -std=gnu99 -O3 -march=native -s -ffast-math
ifneq ($(GLX),)
CFLAGS=-DGLX -DURA -lGL
else
ifneq ($(SDL),)
CFLAGS=-DSDL `sdl-config --cflags` `sdl-config --libs` -lSDL_net -lGL
else
CFLAGS=-lglfw
endif
ifneq ($(URA),)
CFLAGS+=-DURA
endif
endif
vek:vek.c v.h
	${CC} ${CFLAGS} vek.c -o vektorael -fwhole-program
vez:vez.c v.h
	${CC} ${CFLAGS} vez.c -o veziovaer -fwhole-program