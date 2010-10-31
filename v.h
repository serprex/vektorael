#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#ifdef SDL
#include <SDL_net.h>
#else
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#endif
#define pop(x) __builtin_popcount(x)
#define ffs(x) __builtin_ffs(x)
#define case(x) break;case x:;
#define else(x) else if(x)
#define SQR(x) ((x)*(x))
#define W(x,y) (W[y]&(1<<(x)))
#define Wf(x,y) (W[y]^=(1<<(x)))
#ifdef SDL
TCPsocket S;
SDLNet_SocketSet set;
IPaddress ip;
int A;
int any(TCPsocket s){
	SDLNet_CheckSockets(set,1);
	return SDLNet_SocketReady(s);
}
uint8_t readch(){
	uint8_t c;
	A=SDLNet_TCP_Recv(S,&c,1);
	return c;
}
void readx(void*p,int len){SDLNet_TCP_Recv(S,p,len);}
void ship(void*p,int len){SDLNet_TCP_Send(S,p,len);}
#else
int S,A;
int any(int s){
	struct pollfd pfd={.fd=s,.events=POLLIN};
	do s=poll(&pfd,1,1); while(s==-1);
	return s;
}
uint8_t readch(){
	uint8_t c;
	while((A=read(S,&c,1))==-1);
	return c;
}
void readx(void*p,int len){
	do{
		int r;
		while((r=read(S,p,len))==-1);
		p+=r;
		len-=r;
	}while(len);
}
void ship(void*p,int len){
	while(len){
		int nw;
		do nw=write(S,p,len); while(nw<=0);
		p+=nw;
		len-=nw;
	}
}
#endif