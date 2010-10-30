#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <errno.h>
#define pop(x) __builtin_popcount(x)
#define ffs(x) __builtin_ffs(x)
#define case(x) break;case x:;
#define else(x) else if(x)
#define SQR(x) ((x)*(x))
#define W(x,y) (W[y]&(1<<(x)))
#define Wf(x,y) (W[y]^=(1<<(x)))
int S,A;
int any(int s){
	struct pollfd pfd={.fd=s,.events=POLLIN};
	do s=poll(&pfd,1,0); while(s==-1);
	return s;
}
uint8_t readch(){
	uint8_t c;
	while((A=read(S,&c,1))==-1);
	return c;
}
void readx(void*p,int len){
	do{
		while((A=read(S,p,len))==-1);
		p+=A;
		len-=A;
	}while(len);
}
void ship(void*p,int blen){
	while(blen){
		int nw;
		do nw=write(S,p,blen); while(nw<=0);
		p+=nw;
		blen-=nw;
	}
}