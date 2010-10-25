#include "net.h"
#include <sys/time.h>
#include <sys/poll.h>
int S=0;
int any(){
	int r;
	struct pollfd pfd={.fd=S,.events=POLLIN};
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
uint8_t buff[55],blen=0;//2+7*3+32
void shipall(int*c){
	for(int i=0;i<8;i++)
		if(cbts&1<<i&&c[i]!=S){
			uint8_t l=blen;
			void*p=buff;
			while(l){
				int nw;
				do nw=write(c[i],p,l); while(nw<=0);
				p+=nw;
				l-=nw;
			}
		}
	blen=0;
}
void ship(){
	void*p=buff;
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