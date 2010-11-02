#ifdef GLX
#include <GL/glx.h>
#include <sys/time.h>
struct timeval tvx,tvy;
#define KEYSYM XKeycodeToKeysym(dpy,ev.xkey.keycode,0)
#define EV(y) ev.x##y
#else
#include <SDL.h>
#include <SDL_opengl.h>
#include <time.h>
Uint32 tvx,tvy;
#define KEYSYM ev.key.keysym.sym
#define XK_Escape SDLK_ESCAPE
#define KeyPress SDL_KEYDOWN
#define KeyRelease SDL_KEYUP
#define ButtonPress SDL_MOUSEBUTTONDOWN
#define ButtonRelease SDL_MOUSEBUTTONUP
#define MotionNotify SDL_MOUSEMOTION
#define EV(y) ev.y
#endif
#include "v.h"
#include "math.h"
const uint8_t rgb[8][3]={{255,255,255},{255,0,0},{0,255,0},{0,0,255},{0,255,255},{255,255,0},{255,0,255},{128,128,128}};
uint8_t core[4][4],team[8],xy[8][4],buff[20/*2*4+2+5+1+4*/],*bfp,B[32][5],Bs,H[80][6],Hs,D[8][5],Ds,A[80][8],As,chg[8],rad[8],ws[8]={5,1,2,3,4,0,6,7};
uint_fast8_t w,id,cbts;
_Bool mine,mb;
uint16_t W[16];
int mx,my,kda,ksw;
GLuint hud;
void glCirc(int x,int y,int r){
	int r2=r*r,r12=r--*3>>2,xc=0;
	goto skir;
	do{
		if(xc*xc+r*r>=r2)r--;
		skir:
		glVertex2i(x+xc,y+r);
		glVertex2i(x+xc,y-r);
		glVertex2i(x-xc,y+r);
		glVertex2i(x-xc,y-r);
		glVertex2i(x+r,y+xc);
		glVertex2i(x+r,y-xc);
		glVertex2i(x-r,y+xc);
		glVertex2i(x-r,y-xc);
	}while(++xc<=r12);
}
void rplace(){
	for(int i=0;i<256;i++){
		#if RAND_MAX>=65535
		*(uint16_t*)&xy[id][i]=rand();
		#else
		for(int i=0;i<2;i++)xy[id][i]=rand();
		#endif
		if(!W(xy[id][0]>>4,xy[id][1]>>4))break;
	}
}
void die(){
	if(rad[id]!=4)return;
	rad[id]=132;
	int j=0;
	for(int i=0;i<4;i++)
		if(core[i][0]==id+1){
			j=1;
			*bfp++=10;
			*bfp++=i|36;
			core[i][0]=9;
		}
	if(!j)*bfp++=11;
}
#ifdef GLX
void axit(){close(S);}
#endif
void mkwud(){
	int wi[8];
	for(int i=0;i<8;i++)wi[ws[i]]=i<<4;
	glNewList(hud+1,GL_COMPILE);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3ubv(rgb[id]);
	glBegin(GL_LINE_STRIP);
	glVertex2i(wi[0]|1,258);
	glVertex2i(wi[0]|15,258);
	glVertex2i(wi[0]|15,272);
	glVertex2i(wi[0]|3,272);
	glVertex2i(wi[0]|3,260);
	glVertex2i(wi[0]|13,260);
	glVertex2i(wi[0]|13,270);
	glVertex2i(wi[0]|5,270);
	glVertex2i(wi[0]|5,263);
	glVertex2i(wi[0]|11,263);
	glVertex2i(wi[0]|11,266);
	glEnd();
	glBegin(GL_POINTS);
	glCirc(wi[1]|7,265,6);
	glCirc(wi[1]|7,265,4);
	glCirc(wi[2]|7,265,5);
	glCirc(wi[3]|7,265,7);
	glCirc(wi[7]|6,263,4);
	glCirc(wi[7]|10,267,4);
	glVertex2i(wi[3]|7,265);
	glEnd();
	glBegin(GL_LINES);
	glVertex2i(0,256);
	glVertex2i(256,256);
	glVertex2i(wi[2]|8,258);
	glVertex2i(wi[2]|8,265);
	glVertex2i(wi[4]|2,257);
	glVertex2i(wi[4]|14,257);
	glVertex2i(wi[4]|2,271);
	glVertex2i(wi[4]|8,257);
	glVertex2i(wi[4]|8,257);
	glVertex2i(wi[4]|14,271);
	glVertex2i(wi[5]|1,257);
	glVertex2i(wi[5]|15,271);
	glVertex2i(wi[6]|1,257);
	glVertex2i(wi[6]|15,271);
	glVertex2i(wi[6]|15,257);
	glVertex2i(wi[6]|1,271);
	glEnd();
	glEndList();
}
void mkhud(){
	glNewList(hud,GL_COMPILE);
	for(int i=0;i<16;i++)
		for(int j=0;j<16;j++)
			if(W(j,i)){
				glBegin(GL_LINE_LOOP);
				glVertex2i(j<<4|1,i<<4|1);
				glVertex2i(j<<4|15,i<<4|1);
				glVertex2i(j<<4|15,i<<4|15);
				glVertex2i(j<<4|1,i<<4|15);
				glEnd();
			}
	glEndList();
}
int main(int argc,char**argv){
	if(argc<2){
		fprintf(stderr,"ip[port]\n");
		return 1;
	}
	#ifdef SDL
	if(SDL_Init(SDL_INIT_VIDEO)==-1){
		fputs(SDL_GetError(),stderr);
		return 1;
	}
	if(SDLNet_Init()==-1||SDLNet_ResolveHost(&ip,argv[1],argc>2?atoi(argv[2]):2000)==-1||!(S=SDLNet_TCP_Open(&ip))){
		fputs(SDLNet_GetError(),stderr);
		return 1;
	}
	set=SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(set,S);
	#else
	struct sockaddr_in ip={.sin_family=AF_INET,.sin_port=htons(argc>2?atoi(argv[2]):2000)};
	if((S=socket(AF_INET,SOCK_STREAM,0))<0||inet_aton(argv[1],&ip.sin_addr)<=0||connect(S,(struct sockaddr*)&ip,sizeof(ip))<0){
		fprintf(stderr,"%d\n",errno);
		return 1;
	}
	atexit(axit);
	#endif
	FILE*pr=fopen("pr","rb");
	if(pr){
		fread(ws,8,1,pr);
		fclose(pr);
	}
	uint8_t hand[52];
	readx(hand,52);
	id=hand[0];
	cbts=hand[1];
	memcpy(W,hand+2,32);
	for(int i=0;i<4;i++){
		core[i][0]=i&1?hand[34+(i>>1)]>>4:hand[34+(i>>1)]&15;
		memcpy(core[i]+1,hand+36+i*3,3);
	}
	for(int i=0;i<4;i++){
		team[i<<1]=hand[48+i]&15;
		team[i<<1|1]=hand[48+i]>>4;
	}
	#ifdef GLX
	Display*dpy=XOpenDisplay(0);
	XVisualInfo*vi=glXChooseVisual(dpy,DefaultScreen(dpy),(int[]){GLX_RGBA,GLX_DOUBLEBUFFER,None});
	Window win=XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,256,273,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,(XSetWindowAttributes[]){{.colormap=XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone),.event_mask=PointerMotionMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask}});
	Atom wmdel=XInternAtom(dpy,"WM_DELETE_WINDOW",False);
	XSetWMProtocols(dpy,win,&wmdel,1);
	XMapWindow(dpy,win);
	glXMakeCurrent(dpy,win,glXCreateContext(dpy,vi,0,GL_TRUE));
	gettimeofday(&tvx,0);
	srand(tvx.tv_sec);
	#else
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_Surface*dpy=SDL_SetVideoMode(256,273,0,SDL_OPENGL);
	srand(time(0));
	#endif
	glOrtho(0,256,273,0,1,-1);
	hud=glGenLists(2);
	mkwud();
	mkhud();
	rplace();
	float fcx=xy[id][2]=-xy[id][0],fcy=xy[id][3]=-xy[id][1];
	for(;;){
		bfp=buff;
		while(any(S)){
			int r=readch();
			if(r==-1)return 0;
			switch(r&15){
			case(0)//CBTS
				cbts^=1<<(r>>5);
			case(1)//BOMB
				B[Bs][0]=r>>5;
				memcpy(B[Bs]+1,xy[r>>5],2);
				B[Bs][3]=0;
				B[Bs][4]=64;
				Bs++;
			case(2)//NUKE
				B[Bs][0]=r>>5;
				memcpy(B[Bs]+1,xy[r>>5]+2,2);
				B[Bs][3]=0;
				B[Bs][4]=48;
				Bs++;
			case(3)//DOME
				D[Ds][0]=r>>5;
				memcpy(D[Ds]+1,xy[r>>5],2);
				D[Ds][3]=hypot(xy[r>>5][0]-xy[r>>5][2],xy[r>>5][1]-xy[r>>5][3]);
				D[Ds][4]=D[Ds][3]-16;
				Ds++;
			case(4)//WAVE
				A[As][0]=r>>5;
				memcpy(A[As]+1,xy[r>>5],4);
				A[As][5]=7;
				As++;
			case(5)//SHOT
				H[Hs][0]=r>>5;
				memcpy(H[Hs]+1,xy[r>>5],4);
				H[Hs][5]=6;
				Hs++;
			case(6)//WALL
				Wf(xy[r>>5][2],xy[r>>5][3]);
				mkhud();
			case(7)//MINE
				B[Bs][0]=B[Bs+1][0]=r>>5;
				readx(xy,4);
				memcpy(B[Bs]+1,xy,2);
				memcpy(B[Bs+1]+1,xy+2,2);
				B[Bs][3]=B[Bs+1][3]=0;
				B[Bs][4]=B[Bs+1][4]=32;
				Bs+=2;
			case(8)//MOVE
				readx(xy[r>>5],4);
			case(9)//TEAM
				team[r>>5]=readch();
			case(10){//TOOK
				int t=readch();
				core[t&3][0]=t>>2;
			}
			case 11:rad[r>>5]=132;
			case(12)//CORE
				core[team[r>>5]-1][0]=9;
				core[team[r>>5]-1][1]=xy[r>>5][0]&240|8;
				core[team[r>>5]-1][2]=xy[r>>5][1]&240|8;
				core[team[r>>5]-1][3]=xy[r>>5][0]>>4|xy[r>>5][1]&240;
			case(13)//COWI
				r=readch();
				core[r][0]=9;
				core[r][1]=core[r][3]<<4|8;
				core[r][2]=core[r][3]&240|8;
			}
		}
		glCallList(hud+1);
		glCallList(hud);
		glBegin(GL_LINES);
		glVertex2i(w<<4,256);
		glVertex2i(w<<4,273);
		glVertex2i(w+1<<4,273);
		glVertex2i(w+1<<4,256);
		for(int i=0;i<8;i++)
			if(chg[ws[i]]){
				chg[ws[i]]--;
				for(int j=0;j<chg[ws[i]]>>4;j++){
					glVertex2i(i<<4,257+j);
					glVertex2i(i<<4|15,257+j);
				}
				glVertex2i(i<<4,257+(chg[ws[i]]>>4));
				glVertex2i(i<<4|chg[ws[i]]&15,257+(chg[ws[i]]>>4));
			}
		glEnd();
		glBegin(GL_POINTS);
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				glColor3ubv(rgb[i]);
				glVertex2i(xy[i][2],xy[i][3]);
				if(rad[i]!=4){
					if(rad[i]&128){
						if(--rad[i]==127){
							rad[i]=0;
							if(i==id)rplace();
						}
					}else rad[i]++;
				}
				glCirc(xy[i][0],xy[i][1],rad[i]&127);
				if(team[i]){
					glColor3ubv(rgb[team[i]-1]);
					glVertex2i(xy[i][0],xy[i][1]);
				}
			}
		for(int i=0;i<4;i++){
			if(core[i][0]){
				if(core[i][0]!=9){
					core[i][1]=xy[core[i][0]-1][0];
					core[i][2]=xy[core[i][0]-1][1];
					if(core[i][0]==id+1&&(core[i][2]&240|core[i][1]>>4)==core[team[id]-1][3]){
						*bfp++=13;
						*bfp++=i;
						core[i][0]=9;
						core[i][1]=core[i][3]<<4|8;
						core[i][2]=core[i][3]&240|8;
					}
				}else(SQR(xy[id][0]-core[i][1])+SQR(xy[id][1]-core[i][2])<64&&(team[id]!=i+1||(core[i][2]&240|core[i][1]>>4)!=core[i][3])){
					*bfp++=11;
					*bfp++=i|id+1<<2;
					core[i][0]=id+1;
				}
				glColor3ubv(rgb[i]);
				glCirc(core[i][1],core[i][2],2);
				glCirc((core[i][3]&15)<<4|8,core[i][3]&240|8,3);
			}
		}
		for(int i=0;i<Bs;){
			glColor3ubv(rgb[B[i][0]]);
			for(int j=0;j<B[i][3];j+=8)glCirc(B[i][1],B[i][2],j);
			glCirc(B[i][1],B[i][2],B[i][3]);
			if(B[i][3]==B[i][4]&&i<Bs){
				memmove(B+i,B+i+1,(Bs-i)*5);
				Bs--;
				continue;
			}
			if(B[i][3]>8&&SQR(xy[id][0]-B[i][1])+SQR(xy[id][1]-B[i][2])<SQR(B[i][3]))die();
			B[i][3]++;
			i++;
		}
		for(int i=0;i<Ds;){
			glColor3ubv(rgb[D[i][0]]);
			for(int j=D[i][3]-16;j<=D[i][4];j+=2)glCirc(D[i][1],D[i][2],j);
			glCirc(D[i][1],D[i][2],D[i][3]);
			D[i][4]+=2;
			if(D[i][4]==D[i][3]){
				if(SQR(xy[id][0]-D[i][1])+SQR(xy[id][1]-D[i][2])<SQR(D[i][3])&&SQR(xy[id][0]-D[i][1])+SQR(xy[id][1]-D[i][2])>SQR(D[i][3]-16))die();
				memmove(D+i,D+i+1,(Hs-i)*5);
				Ds--;
				continue;
			}
			i++;
		}
		for(int i=0;i<Hs;){
			uint8_t
				xx=H[i][1]+(H[i][3]-H[i][1])*H[i][5]*2/hypot(H[i][3]-H[i][1],H[i][4]-H[i][2]),
				yy=H[i][2]+(H[i][4]-H[i][2])*H[i][5]*2/hypot(H[i][3]-H[i][1],H[i][4]-H[i][2]);
			glColor3ubv(rgb[H[i][0]]);
			glVertex2i(xx,yy);
			if(W(xx>>4,yy>>4)||H[i][5]++==255){
				memmove(H+i,H+i+1,(Hs-i)*6);
				Hs--;
				continue;
			}
			if(SQR(xy[id][0]-xx)+SQR(xy[id][1]-yy)<64)die();
			i++;
		}
		for(int i=0;i<As;){
			uint8_t
				xx=A[i][1]+(A[i][3]-A[i][1])*A[i][5]*1.5/hypot(A[i][3]-A[i][1],A[i][4]-A[i][2]),
				yy=A[i][2]+(A[i][4]-A[i][2])*A[i][5]*1.5/hypot(A[i][3]-A[i][1],A[i][4]-A[i][2]);
			if(A[i][0]&128)xx=(A[i][6]<<1)-xx;
			if(A[i][0]&64)yy=(A[i][7]<<1)-yy;
			glColor3ubv(rgb[A[i][0]&63]);
			glVertex2i(xx,yy);
			if(W(xx>>4,yy>>4)){
				if(A[i][0]&192||A[i][5]++==255)goto killA;
				if((xx&15)<2||(xx&15)>13)A[i][0]|=128;
				if((yy&15)<2||(yy&15)>13)A[i][0]|=64;
				A[i][6]=xx;
				A[i][7]=yy;
			}
			if(A[i][5]++==255){killA:
				memmove(A+i,A+i+1,(As-i)*8);
				As--;
				continue;
			}
			if(SQR(xy[id][0]-xx)+SQR(xy[id][1]-yy)<64)die();
			i++;
		}
		glEnd();
		int k5=0,t=team[id];
		#ifdef GLX
		glXSwapBuffers(dpy,win);
		XEvent ev;
		while(XPending(dpy)){
			KeySym ks;
			XNextEvent(dpy,&ev);
		#else
		SDL_GL_SwapBuffers();
		SDL_Event ev;
		while(SDL_PollEvent(&ev)){
			SDLKey ks;
		#endif
			switch(ev.type){
			case(KeyPress){
				ks=KEYSYM;
				FILE*lv;
				int keq;
				kda+=(ks=='d')-(ks=='a');
				ksw+=(ks=='s')-(ks=='w');
				if(keq=(ks=='e')-(ks=='q')){
					uint8_t t=ws[w];
					ws[w]=ws[w+keq&7];
					ws[w+keq&7]=t;
					w=w+keq&7;
					mkwud();
				}
				if(ks>='0'&&ks<='4')team[id]=ks-'0';
				else(ks=='5')k5=1;
				else(ks==' '){
					if(lv=fopen("lv","wb")){
						fwrite(W,16,2,lv);
						fclose(lv);
					}
					if(lv=fopen("pr","wb")){
						fwrite(ws,8,1,lv);
						fclose(lv);
					}
				}
				else(ks==XK_Escape)return 0;
				else(ks=='p')*bfp++=0;
			}
			case(KeyRelease)
				ks=KEYSYM;
				kda+=(ks=='a')-(ks=='d');
				ksw+=(ks=='w')-(ks=='s');
			case(ButtonPress)
				if(EV(button.button)<4)mb=1;
				else w=w+(EV(button.button)==4)-(EV(button.button)==5)&7;
			case(ButtonRelease)
				if(EV(button.button)<4)mb=0;
			case(MotionNotify)
				mx=EV(motion.x);
				my=EV(motion.y);
		#ifdef GLX
			case(ClientMessage)
				if(ev.xclient.data.l[0]==wmdel)return 0;
		#else
			case(SDL_QUIT)return 0;
		#endif
			}
		}
		if(t!=team[id]){
			*bfp++=9;
			*bfp++=team[id];
		}
		if(xy[id][2]!=mx)xy[id][2]=fcx=fmin(fmax(fcx+(mx-xy[id][2])/hypot(mx-xy[id][2],my-xy[id][3]),0),255);
		if(xy[id][3]!=my)xy[id][3]=fcy=fmin(fmax(fcy+(my-xy[id][3])/hypot(mx-xy[id][2],my-xy[id][3]),0),255);
		xy[id][0]+=kda;
		if(W(xy[id][0]>>4,xy[id][1]>>4))xy[id][0]-=kda;
		xy[id][1]+=ksw;
		if(W(xy[id][0]>>4,xy[id][1]>>4))xy[id][1]-=ksw;
		if(!ws[w]&&!chg[0]&&!W(xy[id][2]>>4,xy[id][3]>>4)&&mb){
			chg[0]=255;
			memcpy(xy[id],xy[id]+2,2);
		}
		*bfp++=8;
		memcpy(bfp,xy+id,4);
		bfp+=4;
		if(k5&&team[id]&&(!core[team[id]-1][0]||(core[team[id]-1][2]&240|core[team[id]-1][1]>>4)!=core[team[id]-1][3])){
			*bfp++=12;
			core[team[id]-1][0]=9;
			core[team[id]-1][1]=xy[id][0]&240|8;
			core[team[id]-1][2]=xy[id][1]&240|8;
			core[team[id]-1][3]=xy[id][0]>>4|xy[id][1]&240;
		}
		if(ws[w]&&!chg[ws[w]]&&mb){
			switch(*bfp++=ws[w]){
			case(1)
				chg[1]=120;
				B[Bs][0]=id;
				memcpy(B[Bs]+1,xy[id],2);
				B[Bs][3]=0;
				B[Bs][4]=64;
				Bs++;
			case(2)
				chg[2]=180;
				B[Bs][0]=id;
				memcpy(B[Bs]+1,xy[id]+2,2);
				B[Bs][3]=0;
				B[Bs][4]=48;
				Bs++;
			case(3)
				chg[3]=150;
				D[Ds][0]=id;
				memcpy(D[Ds]+1,xy[id],2);
				D[Ds][3]=hypot(xy[id][0]-xy[id][2],xy[id][1]-xy[id][3]);
				D[Ds][4]=D[Ds][3]-16;
				Ds++;
			case(4)
				chg[4]=30;
				A[As][0]=id;
				memcpy(A[As]+1,xy[id],4);
				A[As][5]=6;
				As++;
			case(5)
				chg[5]=30;
				H[Hs][0]=id;
				memcpy(H[Hs]+1,xy[id],4);
				H[Hs][5]=5;
				Hs++;
			case(6)
				chg[6]=15;
				Wf(xy[id][2],xy[id][3]);
				mkhud();
			case(7)
				if(mine=!mine){
					chg[7]=15;
					memcpy(bfp,xy[id],4);
					bfp--;
				}else{
					chg[7]=240;
					B[Bs][0]=B[Bs+1][0]=id;
					memcpy(B[Bs]+1,bfp,2);
					memcpy(B[Bs+1]+1,bfp+2,2);
					bfp+=4;
					B[Bs][3]=B[Bs+1][3]=0;
					B[Bs][4]=B[Bs+1][4]=32;
					Bs+=2;
				}
			}
		}
		ship(buff,bfp-buff);
		#ifdef GLX
		gettimeofday(&tvy,0);
		if(tvy.tv_usec>tvx.tv_usec&&tvy.tv_usec-tvx.tv_usec<30000)usleep(33000-(tvy.tv_usec-tvx.tv_usec));
		gettimeofday(&tvx,0);
		#else
		tvy=SDL_GetTicks();
		if(tvy>tvx&&tvy-tvx<30)SDL_Delay(33-(tvy-tvx));
		tvx=SDL_GetTicks();
		#endif
	}
}