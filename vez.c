#include "v.h"
const uint8_t one=1;
#ifdef SDL
TCPsocket lis,con[8];
#else
int lis,con[8];
#endif
uint8_t rgb[8][3],beif[8][256],core[4][4],team[8];
uint_fast8_t cbts,beln[8];
uint16_t W[16];
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
		}else fprintf(stderr,"%d\n",errno);
	}
	#ifdef SDL
	if(SDL_Init(0)==-1){
		fprintf(stderr,"%s\n",SDL_GetError());
		return 1;
	}
	if(SDLNet_Init()==-1||SDLNet_ResolveHost(&ip,0,argc>1?atoi(argv[1]):2000)==-1||!(lis=SDLNet_TCP_Open(&ip))){
		fprintf(stderr,"%s\n",SDLNet_GetError());
		return 1;
	}
	set=SDLNet_AllocSocketSet(9);
	SDLNet_TCP_AddSocket(set,lis);
	#else
	if((lis=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"%d\n",errno);
		return 1;
	}
	setsockopt(lis,SOL_SOCKET,SO_REUSEADDR,&one,1);
	struct sockaddr_in ip={.sin_family=AF_INET,.sin_addr.s_addr=htonl(INADDR_ANY),.sin_port=htons(argc>1?atoi(argv[1]):2000)};
	if(bind(lis,(struct sockaddr*)&ip,sizeof(ip))<0||listen(lis,8)<0){
		fprintf(stderr,"%d\n",errno);
		return 1;
	}
	#endif
	for(;;){
		if(pop(cbts)<8&&any(lis)){
			uint8_t nid=0;
			while(cbts&1<<nid)nid++;
			#ifdef SDL
			if(!(S=con[nid]=SDLNet_TCP_Accept(lis))||SDLNet_TCP_AddSocket(set,S)==-1)fprintf(stderr,"%s\n",SDLNet_GetError());
			#else
			if((S=con[nid]=accept(lis,0,0))<0)fprintf(stderr,"%d\n",errno);
			#endif
			else{
				uint8_t buff[73/*2+3*7+32+14+4*/],*bfp=buff;
				*bfp++=nid;
				*bfp++=cbts|=1<<nid;
				for(int i=0;i<8;i++)
					if(cbts&1<<i&&i!=nid){
						memcpy(bfp,rgb+i,3);
						bfp+=3;
					}
				memcpy(bfp,W,32);
				bfp+=32;
				*bfp++=core[0][0]|core[1][0]<<4;
				*bfp++=core[2][0]|core[3][0]<<4;
				for(int i=0;i<4;i++){
					memcpy(bfp,core[i]+1,3);
					bfp+=3;
				}
				for(int i=0;i<4;i++)*bfp++=team[i<<1]|team[i<<1|1]<<4;
				ship(buff,bfp-buff);
				writech(nid,nid<<5);
				readx(rgb[nid],3);
				writex(nid,rgb[nid],3);
			}
		}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				uint8_t r,xy[4];
				while(any(S=con[i])){
					r=readch();
					if(!A){
						beln[i]=0;
						cbts&=~(1<<i);
						writech(i,i<<5|10);
						#ifdef SDL
						SDLNet_TCP_DelSocket(set,S);
						SDLNet_TCP_Close(S);
						#else
						close(S);
						#endif
						goto nomore;
					}
					writech(i,r&15|i<<5);
					switch(r&15){
					case(6)
						Wf(xy[2]>>4,xy[3]>>4);
					case(7)case 8:
						readx(xy,4);
						writex(i,xy,4);
					case(9)
						writech(i,team[i]=readch());
					case(11)
						writech(i,r=readch());
						core[r&3][0]=r>>2;
					case(12)
						core[team[i]-1][0]=9;
						core[team[i]-1][1]=xy[0]&240|8;
						core[team[i]-1][2]=xy[1]&240|8;
						core[team[i]-1][3]=xy[0]>>4|xy[1]&240;
					case(13)
						writech(i,r=readch());
						core[r][0]=9;
						core[r][1]=core[r][3]<<4|8;
						core[r][2]=core[r][3]&240|8;
					}
				}nomore:;
			}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				#ifdef SDL
				SDLNet_TCP_Send(con[i],beif[i],beln[i]);
				beln[i]=0;
				#else
				void*p=beif[i];
				while(beln[i]){
					int nw;
					while((nw=write(con[i],p,beln[i]))<=0);
					p+=nw;
					beln[i]-=nw;
				}
				#endif
			}
	}
}