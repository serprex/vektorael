#include "net.h"
int lis,con[8];
uint8_t rgb[8][3],x[8],y[8],cx[8],cy[8],cbts;
uint16_t port,W[16];
struct sockaddr_in addr;
int main(int argc,char**argv){
	if(argc<2){
		puts("veziovaer port");
		return 1;
	}
	if(argc>2){
		FILE*lv=fopen(argv[2],"rb");
		if(lv){
			fread(W,16,2,lv);
			fclose(lv);
		}else printf("Failed to load %s\n",argv[2]);
	}
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	port=strtol(argv[1],0,0);
	if((lis=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"vez socket %d\n",errno);
		return 1;
	}
	setsockopt(lis,SOL_SOCKET,SO_REUSEADDR,"\1",1);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	if(bind(lis,(struct sockaddr*)&addr,sizeof(addr))<0){
		fprintf(stderr,"vez bind %d\n",errno);
		return 1;
	}
	if(listen(lis,8)<0){
		fprintf(stderr,"vez listen %d\n",errno);
		return 1;
	}
	for(;;){
		if(pop(cbts)<8&&(S=lis,any())){
			uint8_t nid=0;
			while(cbts&1<<nid)nid++;
			if((S=con[nid]=accept(lis,0,0))<0){
				fprintf(stderr,"vez accept %d\n",errno);
				break;
			}
			writech(nid);
			writech(cbts|=1<<nid);
			for(int i=0;i<8;i++)
				if(cbts&1<<i&&i!=nid){
					writex(rgb[i],3);
				}
			writex(W,32);
			ship();
			writech(nid<<5|7);
			readx(rgb[nid],3);
			writex(rgb[nid],3);
			shipall(con,cbts);
		}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				S=con[i];
				while(any()){
					switch(readch()){
					case(0)
						close(S);
						cbts&=~(1<<i);
						writech(i<<5);
						shipall(con,cbts);
						goto nomore;
					case(1)
						writech(1|i<<5);
						writech(cx[i]=readch());
						writech(cy[i]=readch());
						writech(x[i]=readch());
						writech(y[i]=readch());
					case(2)
						writech(6|i<<5);
					case(3)
						writech(3);
						writech(readch());
						writech(readch());
						writech(3);
						writech(readch());
						writech(readch());
					case(4)
						writech(5|i<<5);
					case(5)
						writech(4|i<<5);
					case(6)
						writech(2|i<<5);
						Wf(cx[i]>>4,cy[i]>>4);
					case(7)
						writech(3);
						writech(x[i]);
						writech(y[i]);
					case(8)
						writech(3);
						writech(cx[i]);
						writech(cy[i]);
					}
					shipall(con,cbts);
				}nomore:;
			}
	}
}