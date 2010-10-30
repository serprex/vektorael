#include "v.h"
int lis,con[8];
uint8_t rgb[8][3],cbts=0,beif[8][256],beln[8];
uint16_t port,W[16];
struct sockaddr_in addr;
void writex(int j,const void*p,int len){
	for(int i=0;i<8;i++)
		if(cbts&1<<i&&i!=j){
			memcpy(beif[i]+beln[i],p,len);
			beln[i]+=len;
		}
}
void writech(int j,uint8_t c){
	for(int i=0;i<8;i++)
		if(cbts&1<<i&&i!=j)
			beif[i][beln[i]++]=c;
}
int main(int argc,char**argv){
	if(argc>2){
		FILE*lv=fopen(argv[2],"rb");
		if(lv){
			fread(W,16,2,lv);
			fclose(lv);
		}else fprintf(stderr,"%s%d\n",argv[2],errno);
	}
	addr.sin_family=AF_INET;
	port=argc>1?strtol(argv[1],0,0):2000;
	if((lis=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"s%d\n",errno);
		return 1;
	}
	setsockopt(lis,SOL_SOCKET,SO_REUSEADDR,"\1",1);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if(bind(lis,(struct sockaddr*)&addr,sizeof(addr))<0){
		fprintf(stderr,"b%d\n",errno);
		return 1;
	}
	if(listen(lis,8)<0){
		fprintf(stderr,"l%d\n",errno);
		return 1;
	}
	for(;;){
		if(pop(cbts)<8&&any(lis)){
			uint8_t nid=0;
			while(cbts&1<<nid)nid++;
			if((S=con[nid]=accept(lis,0,0))<0)fprintf(stderr,"a%d\n",errno);
			else{
				uint8_t buff[55]={nid,cbts|=1<<nid},len=2;
				for(int i=0;i<8;i++)
					if(cbts&1<<i&&i!=nid){
						memcpy(buff+len,rgb+i,3);
						len+=3;
					}
				memcpy(buff+len,W,32);
				ship(buff,len+32);
				writech(nid,nid<<5);
				readx(rgb[nid],3);
				writex(nid,rgb[nid],3);
			}
		}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				while(any(S=con[i])){
					uint8_t r=readch(),xy[4];
					if(!A){
						close(S);
						beln[i]=0;
						cbts&=~(1<<i);
						writech(i,r&15|9);
						goto nomore;
					}
					writech(i,r&15|i<<5);
					switch(r&15){
					case(6)
						Wf(xy[2]>>4,xy[3]>>4);
					case(7)case 8:
						readx(xy,4);
						writex(i,xy,4);
					}
				}nomore:;
			}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				void*p=beif[i];
				while(beln[i]){
					int nw;
					do nw=write(con[i],p,beln[i]); while(nw<=0);
					p+=nw;
					beln[i]-=nw;
				}
			}
	}
}