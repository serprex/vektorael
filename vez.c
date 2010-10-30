#include "net.h"
int lis,con[8];
uint8_t rgb[8][3],x[8],y[8],cx[8],cy[8],cbts=0,buff[55],beif[8][256],beln[8];
uint16_t port,W[16];
struct sockaddr_in addr;
void mvtosh(){
	for(int i=0;i<8;i++)
		if(cbts&1<<i&&con[i]!=S){
			memcpy(beif[i]+beln[i],buff,blen);
			beln[i]+=blen;
		}
	blen=0;
}
void shipall(){
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
int main(int argc,char**argv){
	if(argc<2){
		puts("veziovaer port");
		return 1;
	}else(argc>2){
		FILE*lv=fopen(argv[2],"rb");
		if(lv){
			fread(W,16,2,lv);
			fclose(lv);
		}else fprintf(stderr,"fopen %s %d\n",argv[2],errno);
	}
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	port=strtol(argv[1],0,0);
	if((lis=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"socket %d\n",errno);
		return 1;
	}
	setsockopt(lis,SOL_SOCKET,SO_REUSEADDR,"\1",1);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if(bind(lis,(struct sockaddr*)&addr,sizeof(addr))<0){
		fprintf(stderr,"bind %d\n",errno);
		return 1;
	}
	if(listen(lis,8)<0){
		fprintf(stderr,"listen %d\n",errno);
		return 1;
	}
	for(;;){
		if(pop(cbts)<8&&(S=lis,any())){
			uint8_t nid=0;
			while(cbts&1<<nid)nid++;
			if((S=con[nid]=accept(lis,0,0))<0)fprintf(stderr,"accept %d\n",errno);
			else{
				writech(nid);
				writech(cbts|=1<<nid);
				for(int i=0;i<8;i++)
					if(cbts&1<<i&&i!=nid)writex(rgb[i],3);
				writex(W,32);
				ship();
				writech(nid<<5|6);
				readx(rgb[nid],3);
				writex(rgb[nid],3);
				mvtosh();
			}
		}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				S=con[i];
				while(any()){
					uint8_t r=readch();
					writech(r&15|i<<5);
					switch(r&15){
					case(6)
						Wf(cx[i]>>4,cy[i]>>4);
					case(7)
						writech(readch());
						writech(readch());
						writech(readch());
						writech(readch());
					case(8)
						writech(cx[i]=readch());
						writech(cy[i]=readch());
						writech(x[i]=readch());
						writech(y[i]=readch());
					case(9)
						close(S);
						beln[i]=0;
						cbts&=~(1<<i);
						goto nomore;
					}
				}
				nomore:mvtosh();
			}
		shipall();
	}
}