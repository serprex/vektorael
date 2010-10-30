#include "net.h"
#include <sys/time.h>
#include <sys/poll.h>
int S=0;
uint8_t blen=0;
int any(int s){
	int r;
	struct pollfd pfd={.fd=s,.events=POLLIN};
	do r=poll(&pfd,1,0); while(r==-1);
	return r;
}
uint8_t readch(){
	int r;
	uint8_t c;
	do r=read(S,&c,1); while(r!=1);
	return c;
}
void readx(void*p,int len){
	do{
		int r;
		do r=read(S,p,len); while(r<=0);
		p+=r;
		len-=r;
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
void writech(uint8_t c){
	buff[blen++]=c;
}
void writex(const void*p,int len){
	memcpy(buff+blen,p,len);
	blen+=len;
}