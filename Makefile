v:vek vez
CC=gcc -std=gnu99 -Os -march=native -g -ffast-math -lGL -fwhole-program -o
ifneq ($(GLX),)
CFLAGS=-DGLX
else
CFLAGS=-DSDL `sdl-config --cflags` `sdl-config --libs` -lSDL_net
endif
vek:vek.c v.h
	${CC} vektorael vek.c ${CFLAGS}
vez:vez.c v.h
	${CC} veziovaer vez.c ${CFLAGS}