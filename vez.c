#include "v.h"
#ifdef SDL
TCPsocket lis,con[8];
#else
int lis,con[8];
#endif
uint8_t beif[8][256],core0[4],core[4][3],team[8],beln[8],cbts;
uint16_t W[16];
void writex(int j,const void*p,int len){
	beln[j]-=len;
	for(int i=0;i<8;i++){
		memcpy(beif[i]+beln[i],p,len);
		beln[i]+=len;
	}
}
void writech(uint8_t c){
	for(int i=0;i<8;i++)
		if(con[i]!=S)beif[i][beln[i]++]=c;
}
void core9(int i){
	for(int j=0;j<4;j++)
		if(core0[j]==i+1){
			core0[j]=9;
			break;
		}
}
int main(int argc,char**argv){
	if(argc>2){
		FILE*lv=fopen(argv[2],"rb");
		if(lv){
			fread(W,16,2,lv);
			fclose(lv);
		}
	}
	#ifdef SDL
	if(SDL_Init(0)==-1){
		fputs(SDL_GetError(),stderr);
		return 1;
	}
	if(SDLNet_Init()==-1||SDLNet_ResolveHost(&ip,0,argc>1?atoi(argv[1]):2000)==-1||!(lis=SDLNet_TCP_Open(&ip))||SDLNet_TCP_AddSocket(set=SDLNet_AllocSocketSet(9),lis)==-1){
		fputs(SDLNet_GetError(),stderr);
		return 1;
	}
	#else
	if((lis=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"%d\n",errno);
		return 1;
	}
	static const uint8_t one=1;
	setsockopt(lis,SOL_SOCKET,SO_REUSEADDR,&one,1);
	struct sockaddr_in ip={.sin_family=AF_INET,.sin_addr.s_addr=htonl(INADDR_ANY),.sin_port=htons(argc>1?atoi(argv[1]):2000)};
	if(bind(lis,(void*)&ip,sizeof(ip))<0||listen(lis,8)<0){
		fprintf(stderr,"%d\n",errno);
		return 1;
	}
	#endif
	for(;;){
		if(cbts!=255&&any(lis)){
			uint8_t nid=0;
			while(cbts&1<<nid)nid++;
			#ifdef SDL
			if(!(S=con[nid]=SDLNet_TCP_Accept(lis))||SDLNet_TCP_AddSocket(set,S)==-1)fputs(SDLNet_GetError(),stderr);
			#else
			if((S=con[nid]=accept(lis,0,0))<0)fprintf(stderr,"%d\n",errno);
			#endif
			else{
				uint8_t buff[52];
				memcpy(buff,W,32);
				buff[32]=nid;
				buff[33]=cbts^=1<<nid;
				for(int i=0;i<2;i++)buff[34+i]=core0[i<<1]|core0[i<<1|1]<<4;
				for(int i=0;i<4;i++){
					memcpy(buff+36+i*3,core[i],3);
					buff[48+i]=team[i<<1]|team[i<<1|1]<<4;
				}
				ship(buff,52);
				writech(nid<<5);
			}
		}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				uint8_t xy[4];
				while(any(S=con[i])){
					int r=readch();
					if(r==-1){
						#ifdef SDL
						SDLNet_TCP_DelSocket(set,S);
						SDLNet_TCP_Close(S);
						#else
						close(S);
						#endif
						beln[i]=0;
						cbts^=1<<i;
						writech(i<<5);
						team[i]=0;
						core9(i);
						break;
					}
					writech(r&31|i<<5);
					switch(r&31){
					case(7 ... 8)
						readx(xy,4);
						writex(i,xy,4);
					case(9)writech(team[i]=r>>5);
					case(10)core9(i);
					case(11)
						writech(r=readch());
						Wf(r&15,r>>4);
					case(12)
						core[team[i]-1][0]=xy[0]&240|8;
						core[team[i]-1][1]=xy[1]&240|8;
						core[team[i]-1][2]=xy[0]>>4|xy[1]&240;
						core0[team[i]-1]=9;
					case(13)
						for(int j=0;j<4;j++)
							if(core0[j]==(r>>5)+1){
								core[j][0]=core[j][2]<<4|8;
								core[j][1]=core[j][2]&240|8;
								core0[j]=9;
								break;
							}
					case(14 ... 17)core0[(r&31)-14]=i+1;
					}
				}
			}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				#ifdef SDL
				SDLNet_TCP_Send(con[i],beif[i],beln[i]);
				beln[i]=0;
				#else
				void*p=beif[i];
				while(beln[i]){
					int r;
					while((r=write(con[i],p,beln[i]))==-1);
					p+=r;
					beln[i]-=r;
				}
				#endif
			}
		memset(beln,0,8);
	}
}