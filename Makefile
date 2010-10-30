all:vek vez
CC=gcc -std=gnu99 -Os -march=native -s -ffast-math
ifeq ($(NOLTO),)
CFLAGS=-flto
endif
ifneq ($(GLX),)
CFLAGS+=-DGLX
endif
vek:vek.o net.o
ifeq ($(GLX),)
	${CC} ${CFLAGS} -o vektorael vek.o net.o -lglfw -fwhole-program
else
	${CC} ${CFLAGS} -o vektorael vek.o net.o -lGL -fwhole-program
endif
vez:vez.o net.o
	${CC} ${CFLAGS} -o veziovaer vez.o net.o -fwhole-program