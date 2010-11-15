#include "v.h"
#ifdef SDL
TCPsocket lis,con[8];
#else
int lis,con[8];
#endif
uint8_t beif[8][256],beln[8],cbts,ki[8][8],klim;
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
int gethex(){
	int c;
	switch(c=getchar()){
	case('0'...'9')return c-'0';
	case('A'...'F')return c-'A'+10;
	case('a'...'f')return c-'a'+10;
	}
	return 0;
}
void kill(int i){
	#ifdef SDL
	SDLNet_TCP_DelSocket(set,S);
	SDLNet_TCP_Close(S);
	#else
	close(S);
	#endif
	team[i]=beln[i]=0;
	memset(ki[i],0,8);
	cbts^=1<<i;
	writech(i<<5);
	flag9(i);
}
void won(){
	memset(ki,0,64);
	memset(tar,0,8);
}
void hit(int x,int y){
	switch(mode&7){
	case(0 ... 1)
		ki[y][x]-=mode&1&&ki[y][x];
		if(++ki[x][y]==klim){
			writech(21|x+1<<4);
			won();
		}
	case(2)
		ki[x][y]+=tar[x]==y+1;
	case(3){
		ki[x][y]++;
		int z=0;
		for(int i=0;i<8;i++)z+=!(cbts&1<<i)||ki[x][i];
		if(z==8){
			writech(21|x+1<<5);
			won();
		}
	}
	case(4){
		int z=-1,mxi=0,mxv=0;
		ki[x][y]+=!ki[y][x];
		cbts(i)z++;
		for(int i=0;i<8;i++){
			int k=0;
			for(int j=0;j<8;j++){
				z-=i!=j&&ki[i][j];
				k++;
			}
			if(k>mxv){
				mxv=k;
				mxi=i;
			}
		}
		if(!z){
			writech(21|(mxv?mxi+1<<5:0));
			if(!mxv)writech(255);
			won();
		}
	}
	case(5)
		if(x!=vir&&y!=vir)ki[x][y]=0;
		else{
			ki[x][y]++;
			if(x==vir){
				int z=0;
				for(int i=0;i<8;i++)z+=!(cbts&1<<i)||ki[x][i];
				if(z==8){
					writech(21|x+1<<5);
					won();
				}
			}else{
				int z=0;
				for(int i=0;i<8;i++)z+=!(cbts&1<<i)||ki[i][y];
				if(z==8){
					writech(21);
					writech(255^1<<y);
					won();
				}
			}
		}
	case(6)
		if(vir==y)ki[x][y]++;
		else(vir==x)writech(22|(vir=y)<<5);
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
				uint8_t*buff=beif[nid];
				memcpy(buff,W,32);
				buff[32]=nid;
				buff[33]=cbts^=1<<nid;
				for(int i=0;i<2;i++)buff[34+i]=flag0[i<<1]|flag0[i<<1|1]<<4;
				for(int i=0;i<4;i++){
					memcpy(buff+36+i*3,flag[i],3);
					buff[48+i]=team[i<<1]|team[i<<1|1]<<4;
				}
				ship(buff,52);
				writech(nid<<5);
			}
		}
		#ifdef GLX
		while(any(S=0)){
			switch(getchar()){
			uint8_t x,y;
			default:fprintf(stderr,"?\n");
			case('i')
				fprintf(stderr,"cbts %x\nmode %x\nviru %x\ntarg %.16lx\nteam %.16lx\nflag %.8x\n",cbts,mode,vir,*(uint64_t*)tar,*(uint64_t*)team,*(uint32_t*)flag0);
				for(int i=0;i<8;i++)fprintf(stderr,"%.16lx\n",*(uint64_t*)ki[i]);
			case('k')kill(gethex());
			case('l')
				klim=gethex();
				if(any(0))klim=klim<<4|gethex();
			case('m')
				mode=gethex();
				if(any(0))mode=mode<<4|gethex();
				writech(18);
				writech(mode);
				memset(ki,0,64);
			case('r')
				writech(20);
				won();
			case('v')writech(22|(vir=gethex())<<5);
			case('w')
				x=gethex();
				y=gethex();
				Wf(x,y);
				writech(11);
				writech(x|y<<4);
			case('x')
				x=gethex();
				tar[x]=y=gethex();
				writech(19);
				writech(x|y<<4);
			}
		}
		#endif
		cbts(i){
			uint8_t xy[4];
			while(any(S=con[i])){
				int r=readch();
				if(r==-1){
					kill(i);
					break;
				}
				writech(r&31|i<<5);
				switch(r&31){
				case(7 ... 8)
					readx(xy,4);
					writex(i,xy,4);
				case(9)writech(team[i]=r>>5);
				case(10)
					flag9(i);
					r=readch()&7;
					if(!(mode&24))hit(r,i);
					if(mode&16&&team[r]&&team[i])hit(team[r]-1,team[i]-1);
				case(11)
					writech(r=readch());
					Wf(r&15,r>>4);
				case(12)
					flag[team[i]-1][0]=xy[0]&240|8;
					flag[team[i]-1][1]=xy[1]&240|8;
					flag[team[i]-1][2]=xy[0]>>4|xy[1]&240;
					flag0[team[i]-1]=9;
				case(13)
					for(int j=0;j<4;j++)
						if(flag0[j]==(r>>5)+1){
							flag[j][0]=flag[j][2]<<4|8;
							flag[j][1]=flag[j][2]&240|8;
							flag0[j]=9;
							if(mode&8)hit(team[r>>5]-1,j);
							break;
						}
				case(14 ... 17)flag0[(r&31)-14]=i+1;
				}
			}
		}
		cbts(i){
			#ifdef SDL
			SDLNet_TCP_Send(con[i],beif[i],beln[i]);
			#else
			S=con[i];
			ship(beif[i],beln[i]);
			#endif
			beln[i]=0;
		}
		memset(beln,0,8);
	}
}