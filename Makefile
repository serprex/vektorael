all:vek vez
CC=gcc -std=gnu99 -O3 -march=native -s -ffast-math
ifneq ($(GLX),)
CFLAGS=-DGLX -lGL
else
CFLAGS=-lglfw
endif
vek:vek.c v.h
	${CC} ${CFLAGS} vek.c -o vektorael -fwhole-program
vez:vez.c v.h
	${CC} ${CFLAGS} vez.c -o veziovaer -fwhole-program