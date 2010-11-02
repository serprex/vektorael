v:vek vez
CC=gcc -std=gnu99 -O3 -march=native -s -ffast-math -lGL -fwhole-program -o
ifneq ($(GLX),)
CFLAGS=-DGLX
else
CFLAGS=-DSDL `sdl-config --cflags` `sdl-config --libs` -lSDL_net
endif
vek:vek.c v.h
	${CC} vektorael vek.c ${CFLAGS}
vez:vez.c v.h
	${CC} veziovaer vez.c ${CFLAGS}