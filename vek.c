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
#define KeyPress SDL_KEYDOWN
#define KeyRelease SDL_KEYUP
#define ButtonPress SDL_MOUSEBUTTONDOWN
#define ButtonRelease SDL_MOUSEBUTTONUP
#define MotionNotify SDL_MOUSEMOTION
#define EV(y) ev.y
#endif
#include "v.h"
#include <math.h>
const uint8_t rgb[8][3]={{255,255,255},{255,64,64},{64,255,64},{0,255,255},{255,0,255},{255,255,0},{64,64,255},{112,112,112}};
uint8_t core0[4],core[4][3],team[8],xy[8][4],buff[15],mixy[4],*bfp,B[40][5],Bs,A[160][8],As,chg[8],rad[8],ws[8]={5,4,2,3,1,7,0,6};
uint_fast8_t w,id,cbts,mx,my;
int_fast8_t kda,ksw;
_Bool mine,mb;
uint16_t W[16],xyv[1520]__attribute__((aligned(16)));
uint32_t reed;
GLuint hud;
void core9(int i){
	for(int j=0;j<4;j++)
		if(core0[j]==i+1){
			core0[j]=9;
			break;
		}
}
void glCirc(int x,int y,int r){
	uint16_t*xyp=xyv,r2=r*r,r12=r--*3>>2,rr=-1,xc=0;
	goto skir;
	do{
		if(r*r>=r2)r--;
		skir:
		for(int i=0;i<8;i++){
			*xyp++=x+(i<4?(i&2?-xc:xc):(i&2?-r:r));
			*xyp++=y+(i<4?(i&1?-r:r):(i&1?-xc:xc));
		}
		r2-=rr+=2;
	}while(++xc<=r12);
	glDrawArrays(GL_POINTS,0,xyp-xyv>>1);
}
void rplace(){
	for(int i=0;i<256;i++){
		*(uint16_t*)(xy+id)^=reed=(reed^13)>>3|(reed^13)<<29;
		if(!W(xy[id][0]>>4,xy[id][1]>>4))break;
	}
}
void die(){
	if(rad[id]!=64)return;
	rad[id]=192;
	*bfp++=10;
	memset(chg,255,8);
	core9(id);
}
void mkhud(){
	uint_fast8_t wi[8];
	for(int i=0;i<8;i++)wi[ws[i]]=i<<4;
	for(int i=0;i<13;i++){
		xyv[i*2]=wi[0]|1+(i+1&2?14-(i+1>>1):i+1>>1);
		xyv[i*2+1]=258+(i&2?14-(i>>1):i>>1);
	}
	glNewList(hud,GL_COMPILE);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3ubv(rgb[id]);
	glDrawArrays(GL_LINE_STRIP,0,13);
	for(int i=4;i<7;i++)glCirc(wi[i&1?3:2]|8,265,i);
	xyv[0]=wi[1]|8;
	xyv[1]=265;
	glDrawArrays(GL_POINTS,0,1);
	for(int i=0;i<3;i++)glCirc(wi[i&1?1:7]|6+(i<<1),263+(i<<1),i&1?7:4);
	glBegin(GL_LINES);
	for(int i=0;i<2;i++)glVertex2i(i<<8,256);
	static const uint8_t ppack[]={1,2,8,9,14,15},pack[]={17,19,72,104,77,80,80,101,128,173,192,236,232,197};
	for(int i=0;i<sizeof(pack);i++)glVertex2i(wi[3+(pack[i]>>6)]|ppack[pack[i]>>3&7],256|ppack[pack[i]&7]);
	glEnd();
	for(int i=0;i<16;i++)
		for(int j=0;j<16;j++)
			if(W(i,j)){
				glBegin(GL_LINE_LOOP);
				for(int k=0;k<4;k++)glVertex2i(i<<4|(k+1&2?1:15),j<<4|(k&2?1:15));
				glEnd();
			}
	glEndList();
}
int mkwep(int w,int id,int c){
	static const uint8_t ch[]={120,180,150,30,30,255};
	if(c)chg[w]=ch[w-1];
	switch(w){
	default:__builtin_unreachable();
	case(1)
		B[Bs][4]=sqrt(SQR(xy[id][0]-xy[id][2])+SQR(xy[id][1]-xy[id][3]));
		for(int i=0;i<2;i++)
			if(B[Bs][4]>xy[id][i])B[Bs][4]=xy[id][i];
			else(B[Bs][4]>255-xy[id][i])B[Bs][4]=255-xy[id][i];
		if(B[Bs][4]>16){
			B[Bs][0]=128|id;
			memcpy(B[Bs]+1,xy[id],2);
			B[Bs][3]=B[Bs][4]-16;
			Bs++;
		}else return 0;
	case(2)case 3:
		B[Bs][0]=id;
		memcpy(B[Bs]+1,xy[id]+(w-2<<1),2);
		B[Bs][3]=0;
		B[Bs][4]=40+(w<<3);
		Bs++;
	case(4)case 5:
		if(*(uint16_t*)xy[id]!=*(uint16_t*)(xy[id]+2)){
			A[As][0]=id|(w-4)<<5;
			memcpy(A[As]+1,xy[id],4);
			A[As][5]=7;
			As++;
		}else return 0;
	case(6)
		rad[id]=16;
		core9(id);
	}
	return 1;
}
int main(int argc,char**argv){
	if(argc<2){
		fprintf(stderr,"ip port\n");
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
	SDLNet_TCP_AddSocket(set=SDLNet_AllocSocketSet(1),S);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_Surface*dpy=SDL_SetVideoMode(256,272,0,SDL_OPENGL);
	reed=time(0)^time(0)<<4^time(0)<<12;
	#else
	struct sockaddr_in ip={.sin_family=AF_INET,.sin_port=htons(argc>2?atoi(argv[2]):2000)};
	if((S=socket(AF_INET,SOCK_STREAM,0))<0||inet_aton(argv[1],&ip.sin_addr)<=0||connect(S,(struct sockaddr*)&ip,sizeof(ip))<0){
		fprintf(stderr,"%d\n",errno);
		return 1;
	}
	Display*dpy=XOpenDisplay(0);
	XVisualInfo*vi=glXChooseVisual(dpy,DefaultScreen(dpy),(int[]){GLX_RGBA,GLX_DOUBLEBUFFER,None});
	Window win=XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,256,272,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,(XSetWindowAttributes[]){{.colormap=XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone),.event_mask=PointerMotionMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask}});
	Atom wmdel=XInternAtom(dpy,"WM_DELETE_WINDOW",False);
	XSetWMProtocols(dpy,win,&wmdel,1);
	XMapWindow(dpy,win);
	glXMakeCurrent(dpy,win,glXCreateContext(dpy,vi,0,GL_TRUE));
	gettimeofday(&tvx,0);
	reed=tvx.tv_usec;
	#endif
	FILE*pr=fopen("pr","rb");
	if(pr){
		fseek(pr,32,SEEK_SET);
		fread(ws,8,1,pr);
		fclose(pr);
	}
	uint8_t*hand=(uint8_t*)xyv;
	readx(hand,52);
	memcpy(W,hand,32);
	id=hand[32];
	cbts=hand[33];
	for(int i=0;i<4;i++){
		core0[i]=i&1?hand[34+(i>>1)]>>4:hand[34+(i>>1)]&15;
		memcpy(core[i],hand+36+i*3,3);
	}
	for(int i=0;i<8;i++)team[i]=i&1?hand[48+(i>>1)]>>4:hand[48+(i>>1)]&15;
	glOrtho(0,255,272,0,1,-1);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2,GL_SHORT,0,xyv);
	hud=glGenLists(1);
	mkhud();
	rplace();
	float fcx=xy[id][2]=-xy[id][0],fcy=xy[id][3]=-xy[id][1];
	for(;;){
		bfp=buff;
		while(any(S)){
			int r=readch(),r5=r>>5;
			if(r==-1)return 0;
			switch(r&31){
			case(0)
				cbts^=1<<(r5);
				if(!(cbts&1<<r5)){
					team[r5]=0;
					core9(r5);
				}
			case(1 ... 6)mkwep(r&31,r5,0);
			case(7)
				B[Bs][0]=B[Bs+1][0]=r5;
				readx(bfp,4);
				memcpy(B[Bs]+1,bfp,2);
				memcpy(B[Bs+1]+1,bfp+2,2);
				*(uint16_t*)(B[Bs]+3)=*(uint16_t*)(B[Bs+1]+3)=0x3000;
				Bs+=2;
			case(8)readx(xy[r5],4);
			case(9)team[r5]=readch();
			case(10)
				rad[r5]=192;
				core9(r5);
			case(11)
				r=readch();
				Wf(r&15,r>>4);
				mkhud();
			case(12)
				core0[team[r5]-1]=9;
				core[team[r5]-1][0]=xy[r5][0]&240|8;
				core[team[r5]-1][1]=xy[r5][1]&240|8;
				core[team[r5]-1][2]=xy[r5][0]>>4|xy[r5][1]&240;
			case(13)
				for(int j=0;j<4;j++)
					if(core0[j]==r5+1){
						core0[j]=9;
						core[j][0]=core[j][2]<<4|8;
						core[j][1]=core[j][2]&240|8;
						break;
					}
			case(14 ... 17)core0[(r&31)-14]=r5+1;
			}
		}
		glCallList(hud);
		glBegin(GL_LINES);
		for(int i=0;i<4;i++)glVertex2i(w+(i>>1)<<4,i+1&2?272:256);
		for(int i=0;i<8;i++)
			if(chg[ws[i]]){
				chg[ws[i]]--;
				for(int j=0;j<chg[ws[i]]>>4;j++)
					for(int k=0;k<2;k++)glVertex2i(i+k<<4,257+j);
				for(int j=0;j<2;j++)glVertex2i(i<<4|(j?chg[ws[i]]&15:0),257+(chg[ws[i]]>>4));
			}
		glEnd();
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				glColor3ubv(rgb[i]);
				glCirc(xy[i][0],xy[i][1],(rad[i]&127)>>4);
				glBegin(GL_POINTS);
				glVertex2i(xy[i][2],xy[i][3]);
				if(team[i]){
					glColor3ubv(rgb[team[i]-1]);
					glVertex2i(xy[i][0],xy[i][1]);
				}
				glEnd();
				if(rad[i]!=64){
					if(rad[i]&128){
						if(--rad[i]==144){
							rad[i]=16;
							if(i==id)rplace();
						}
					}else rad[i]++;
				}
			}
		for(int i=0;i<4;i++)
			if(core0[i]){
				if(core0[i]!=9){
					memcpy(core[i],xy[core0[i]-1],2);
					if(core0[i]==id+1&&(core[i][1]&240|core[i][0]>>4)==core[team[id]-1][2]){
						*bfp++=13;
						core0[i]=9;
						core[i][0]=core[i][2]<<4|8;
						core[i][1]=core[i][2]&240|8;
					}
				}else(SQR(xy[id][0]-core[i][0])+SQR(xy[id][1]-core[i][1])<64&&rad[id]==64&&(team[id]!=i+1||(core[i][1]&240|core[i][0]>>4)!=core[i][2])){
					for(int i=0;i<4;i++)
						if(core0[i]==id+1)goto noj;
					*bfp++=14+i;
					core0[i]=id+1;
				}
				noj:glColor3ubv(rgb[i]);
				glCirc(core[i][0],core[i][1],2);
				glCirc((core[i][2]&15)<<4|8,core[i][2]&240|8,3);
			}
		for(int i=0;i<Bs;){
			glColor3ubv(rgb[B[i][0]&127]);
			int b=B[i][0]&128,sqr=SQR(xy[id][0]-B[i][1])+SQR(xy[id][1]-B[i][2]);
			for(int j=b?B[i][4]-16:0;j<=B[i][3];j+=b?3:8)glCirc(B[i][1],B[i][2],j);
			glCirc(B[i][1],B[i][2],B[i][b?4:3]);
			if(B[i][3]==B[i][4]){
				if(b&&sqr<SQR(B[i][4])&&sqr>SQR(B[i][4]-16))die();
				memmove(B+i,B+i+1,(Bs-i)*5);
				Bs--;
				continue;
			}else(!b&&B[i][3]&&sqr<SQR(B[i][3]))die();
			B[i][3]+=b?2:1;
			i++;
		}
		if(As){
			uint8_t ahco[160][3];
			for(int i=0;i<As;){
				uint8_t
					xx=A[i][1]+(A[i][3]-A[i][1])*(A[i][0]&32?A[i][5]<<1:A[i][5]*3>>1)/sqrt(SQR(A[i][3]-A[i][1])+SQR(A[i][4]-A[i][2])),
					yy=A[i][2]+(A[i][4]-A[i][2])*(A[i][0]&32?A[i][5]<<1:A[i][5]*3>>1)/sqrt(SQR(A[i][3]-A[i][1])+SQR(A[i][4]-A[i][2]));
				if(A[i][0]&128)xx=(A[i][6]<<1)-xx;
				if(A[i][0]&64)yy=(A[i][7]<<1)-yy;
				memcpy(ahco[i],rgb[A[i][0]&7],3);
				xyv[i*2]=xx;
				xyv[i*2+1]=yy;
				if(W(xx>>4,yy>>4)){
					if(A[i][0]&224||A[i][5]++==255)goto killA;
					if(!((xx&15)<3?W((xx>>4)-1&15,yy>>4):(xx&15)>12?W((xx>>4)+1&15,yy>>4):1))A[i][0]|=128;
					if(!((yy&15)<3?W(xx>>4,(yy>>4)-1&15):(yy&15)>12?W(xx>>4,(yy>>4)+1&15):1))A[i][0]|=64;
					memcpy(A[i]+6,xyv+i*2,2);
				}
				if(A[i][5]++==255){killA:
					memmove(A+i,A+i+1,(As-i)*8);
					As--;
					continue;
				}
				if(SQR(xy[id][0]-xx)+SQR(xy[id][1]-yy)<64)die();
				i++;
			}
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3,GL_UNSIGNED_BYTE,0,ahco);
			glDrawArrays(GL_POINTS,0,As);
			glDisableClientState(GL_COLOR_ARRAY);
		}
		int k5=0,t=team[id],wall=0;
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
			case(MotionNotify)
				mx=EV(motion.x);
				if(EV(motion.y)<256)my=EV(motion.y);
			case(KeyPress)case KeyRelease:{
				ks=KEYSYM;
				kda+=((ks=='d')-(ks=='a'))*(ev.type==KeyPress?:-1);
				ksw+=((ks=='s')-(ks=='w'))*(ev.type==KeyPress?:-1);
				if(ev.type==KeyPress)w=w+(ks=='m')-(ks=='n')&7;
				int keq;
				if(ks=='b')mb=ev.type==KeyPress;
				else(ev.type==KeyRelease)break;
				else(keq=(ks=='e')-(ks=='q')){
					uint8_t t=ws[w];
					ws[w]=ws[w+keq&7];
					ws[w+keq&7]=t;
					w=w+keq&7;
					mkhud();
				}else(ks>='0'&&ks<='4')team[id]=ks-'0';
				else(ks=='5')k5=1;
				else(ks==' '){
					FILE*lv=fopen("pr","wb");
					if(lv){
						fwrite(W,16,2,lv);
						fwrite(ws,8,1,lv);
						fclose(lv);
					}
				}
			}
			case(ButtonPress)
				if(EV(button.button)==1)case ButtonRelease:mb=ev.type==ButtonPress;
				else(EV(button.button)==3&&!wall){
					wall=1;
					*bfp++=11;
					*bfp++=mx>>4|my&240;
					Wf(mx>>4,my>>4);
					mkhud();
				}else w=w+(EV(button.button)==4)-(EV(button.button)==5)&7;
		#ifdef GLX
			case(ClientMessage)
				if(ev.xclient.data.l[0]==wmdel)return 0;
		#else
			case(SDL_QUIT)return 0;
		#endif
			}
		}
		if(t!=team[id])*bfp++=9|team[id]<<5;
		if(xy[id][2]!=mx)xy[id][2]=fcx+=(mx-xy[id][2])/sqrt(SQR(mx-xy[id][2])+SQR(my-xy[id][3]));
		if(xy[id][3]!=my)xy[id][3]=fcy+=(my-xy[id][3])/sqrt(SQR(mx-xy[id][2])+SQR(my-xy[id][3]));
		if(!W(xy[id][0]+kda>>4,xy[id][1]>>4))xy[id][0]+=kda;
		if(!W(xy[id][0]>>4,xy[id][1]+ksw>>4))xy[id][1]+=ksw;
		if(!ws[w]&&mb&&!chg[0]&&!W(xy[id][2]>>4,xy[id][3]>>4)){
			chg[0]=255;
			memcpy(xy[id],xy[id]+2,2);
		}
		*bfp++=8;
		memcpy(bfp,xy+id,4);
		bfp+=4;
		if(k5&&team[id]&&(!core0[team[id]-1]||(core[team[id]-1][1]&240|core[team[id]-1][0]>>4)!=core[team[id]-1][2])){
			*bfp++=12;
			core0[team[id]-1]=9;
			core[team[id]-1][0]=xy[id][0]&240|8;
			core[team[id]-1][1]=xy[id][1]&240|8;
			core[team[id]-1][2]=xy[id][0]>>4|xy[id][1]&240;
		}
		if(mb&&ws[w]&&!chg[ws[w]]){
			if((*bfp=ws[w])<7)bfp+=mkwep(ws[w],id,1);
			else(mine=!mine){
				chg[7]=15;
				memcpy(mixy,xy[id],4);
			}else{
				chg[7]=180;
				B[Bs][0]=B[Bs+1][0]=id;
				*bfp++=7;
				memcpy(bfp,mixy,4);
				bfp+=4;
				memcpy(B[Bs]+1,mixy,2);
				memcpy(B[Bs+1]+1,mixy+2,2);
				B[Bs][3]=B[Bs+1][3]=0;
				B[Bs][4]=B[Bs+1][4]=48;
				Bs+=2;
			}
		}
		ship(buff,bfp-buff);
		#ifdef GLX
		gettimeofday(&tvy,0);
		if(tvy.tv_usec>tvx.tv_usec&&tvy.tv_usec-tvx.tv_usec<30000)usleep(33000-(tvy.tv_usec-tvx.tv_usec));
		gettimeofday(&tvx,0);
		reed+=tvx.tv_usec;
		#else
		tvy=SDL_GetTicks();
		if(tvy>tvx&&tvy-tvx<30)SDL_Delay(33-(tvy-tvx));
		reed+=tvx=SDL_GetTicks();
		#endif
	}
}