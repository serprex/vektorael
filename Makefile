CC=gcc -std=gnu99 -flto -Os -g
all:vek vez
vek:vek.o net.o
	${CC} -o vektorael vek.o net.o -lglfw -fwhole-program
vez:vez.o net.o
	${CC} -o veziovaer vez.o net.o -fwhole-program