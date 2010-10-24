#pragma once
#define __GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#define pop(x) __builtin_popcount(x)
#define ffs(x) __builtin_ffs(x)
#define case(x) break;case x:
#define else(x) else if(x)
uint8_t readch(void);
void readx(char*,int);
int readln(char*,int);
void writech(uint8_t);
void writex(const void*,int);
void writeln(const void*);
int any(void);
void ship(void);
void shipall(int*,uint8_t);
extern int S;