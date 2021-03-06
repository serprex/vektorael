#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#ifdef SDL
#include <SDL_net.h>
#else
#include <errno.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#endif
#define case(x) break;case x:;
#define else(x) else if(x)
#define SQR(x) ((x)*(x))
#define W(x,y) (W[y]&(1<<(x)))
#define Wf(x,y) (W[y]^=(1<<(x)))
#define cbts(x) for(int x=0;x<8;x++)if(cbts&1<<x)
uint8_t flag0[4],flag[4][3],team[8],tar[8],mode,vir;
void flag9(int i){
	for(int j=0;j<4;j++)
		if(flag0[j]==i+1){
			flag0[j]=9;
			break;
		}
}
#ifdef SDL
TCPsocket S;
SDLNet_SocketSet set;
IPaddress ip;
int any(TCPsocket s){
	SDLNet_CheckSockets(set,1);
	return SDLNet_SocketReady(s);
}
int readch(){
	uint8_t c;
	return SDLNet_TCP_Recv(S,&c,1)?c:-1;
}
void readx(void*p,int len){SDLNet_TCP_Recv(S,p,len);}
void ship(void*p,int len){SDLNet_TCP_Send(S,p,len);}
#else
int S;
int any(int s){
	struct pollfd pfd={.fd=s,.events=POLLIN};
	while((s=poll(&pfd,1,1))==-1);
	return s;
}
int readch(){
	uint8_t c;
	ssize_t A;
	while((A=read(S,&c,1))==-1);
	return A?c:-1;
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
	do{
		int r;
		while((r=write(S,p,len))==-1);
		p+=r;
		len-=r;
	}while(len);
}
#endif