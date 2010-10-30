#include "net.h"
#include <sys/time.h>
#include <sys/poll.h>
int S=0;
int any(int s){
	struct pollfd pfd={.fd=s,.events=POLLIN};
	do s=poll(&pfd,1,0); while(s==-1);
	return s;
}
uint8_t readch(){
	uint8_t c;
	while(read(S,&c,1)!=1);
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