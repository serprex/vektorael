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
#define SQR(x) ((x)*(x))
#define W(x,y) (W[y]&(1<<(x)))
#define Wf(x,y) (W[y]^=(1<<(x)))
uint8_t readch(void);
void readx(void*,int);
void writech(uint8_t);
void writex(const void*,int);
int any(void);
void ship(void);
void shipall(int*);
extern uint8_t cbts;
extern int S;
extern uint8_t blen;