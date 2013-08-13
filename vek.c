#include <math.h>
#ifdef GLX
#include <GL/glx.h>
#include <sys/time.h>
struct timeval tvx,tvy;
#define KEYSYM XKeycodeToKeysym(dpy,ev.xkey.keycode,0)
#define EV(y) ev.x##y
#define SDL_SCANCODE_SPACE ' '
#define SDL_SCANCODE_A 'a'
#define SDL_SCANCODE_B 'b'
#define SDL_SCANCODE_D 'd'
#define SDL_SCANCODE_E 'e'
#define SDL_SCANCODE_M 'm'
#define SDL_SCANCODE_N 'n'
#define SDL_SCANCODE_P 'p'
#define SDL_SCANCODE_Q 'q'
#define SDL_SCANCODE_S 's'
#define SDL_SCANCODE_W 'w'
#define SDL_SCANCODE_0 '0'
#define SDL_SCANCODE_4 '4'
#define SDL_SCANCODE_5 '5'
#else
#include <SDL.h>
#include <SDL_opengl.h>
#include <time.h>
Uint32 tvx,tvy;
#define KEYSYM (ev.key.keysym.scancode)
#define KeyPress SDL_KEYDOWN
#define KeyRelease SDL_KEYUP
#define ButtonPress SDL_MOUSEBUTTONDOWN
#define ButtonRelease SDL_MOUSEBUTTONUP
#define MotionNotify SDL_MOUSEMOTION
#define ClientMessage SDL_QUIT
#define EV(y) ev.y
#endif
#include "v.h"
const uint8_t rgb[8][3]={{255,255,255},{255,64,64},{64,255,64},{0,255,255},{255,0,255},{255,255,0},{64,64,255},{112,112,112}};
uint8_t*bfp,buff[16],xy[8][4],B[40][5],Bs,A[160][8],As,chg[8],rad[8],win[8],ws[8]={5,4,2,3,1,7,0,6};
uint_fast8_t w,id,cbts,mx,my;
int_fast8_t kda,ksw;
_Bool mine,mb,sbo;
uint16_t W[16],xyv[1520]__attribute__((aligned(16)));
GLuint hud;
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
void die(int i){
	if(rad[id]!=64)return;
	rad[id]=192;
	*bfp++=10|i<<5;
	memset(chg,255,8);
	flag9(id);
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
	for(int i=6;i<12;i+=2)glCirc(wi[i&2?7:1]|i,257+i,i&2?4:7);
	glBegin(GL_LINES);
	for(int i=0;i<2;i++)glVertex2i(i<<8,256);
	static const uint8_t ppack[]={1,2,8,9,14,15},pack[]={25,27,72,104,77,80,80,101,128,173,192,236,232,197};
	for(int i=0;i<sizeof(pack);i++)glVertex2i(wi[3+(pack[i]>>6)]|ppack[pack[i]>>3&7],256|ppack[pack[i]&7]);
	glEnd();
	for(int i=0;i<16;i++)
		for(int j=0;j<16;j++)
			if(W(i,j)){
				for(int k=0;k<8;k++)xyv[k]=(k&1?j:i)<<4|(k+1&4?1:15);
				glDrawArrays(GL_LINE_LOOP,0,4);
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
		B[Bs][4]=w==2?64:48;
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
		flag9(id);
	}
	return 1;
}
int aimv(int da,int sw,uint8_t x,uint8_t y,int t){
	for(int i=0;i<As;i++)
		if(A[i][5]+t<256){
			uint8_t
				xx=A[i][1]+(A[i][3]-A[i][1])*(A[i][0]&32?(A[i][5]+t)<<1:(A[i][5]+t)*3>>1)/sqrt(SQR(A[i][3]-A[i][1])+SQR(A[i][4]-A[i][2])),
				yy=A[i][2]+(A[i][4]-A[i][2])*(A[i][0]&32?(A[i][5]+t)<<1:(A[i][5]+t)*3>>1)/sqrt(SQR(A[i][3]-A[i][1])+SQR(A[i][4]-A[i][2]));
			if(A[i][0]&128)xx=(A[i][6]<<1)-xx;
			if(A[i][0]&64)yy=(A[i][7]<<1)-yy;
			if(SQR(xx-x)+SQR(yy-y)<64)return 0;
		}
	for(int i=0;i<Bs;i++)
		if(B[i][3]+t<=B[i][4]){
			if(B[i][0]&128){
				if(SQR(B[i][1]-x)+SQR(B[i][2]-y)>=SQR(B[i][3]+t))return 0;
			}else(SQR(B[i][1]-x)+SQR(B[i][2]-y)<=SQR(B[i][3]+t))return 0;
		}
	if(da&&W((x+da&255)>>4,y)){
		if(!sw)sw=da;
		da=0;
	}
	if(sw&&W(x,(y+sw&255)>>4)){
		if(!da)da=sw;
		sw=0;
	}
	return t++==72?:1+(da||sw?aimv(da,sw,x+da,y+sw,t):0)+aimv(0,0,x,y,t);
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
		fputs(SDL_GetError(),stderr);
		return 1;
	}
	SDLNet_TCP_AddSocket(set=SDLNet_AllocSocketSet(1),S);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_Window*dpy=SDL_CreateWindow(0,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,256,272,SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(dpy);
	#else
	struct sockaddr_in ip={.sin_family=AF_INET,.sin_port=htons(argc>2?atoi(argv[2]):2000)};
	if((S=socket(AF_INET,SOCK_STREAM,0))<0||inet_aton(argv[1],&ip.sin_addr)<=0||connect(S,(struct sockaddr*)&ip,sizeof(ip))<0){
		fprintf(stderr,"%d\n",errno);
		return 1;
	}
	Display*dpy=XOpenDisplay(0);
	XVisualInfo*vi=glXChooseVisual(dpy,DefaultScreen(dpy),(int[]){GLX_RGBA,GLX_DOUBLEBUFFER,None});
	Window Wdo=XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,256,272,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,(XSetWindowAttributes[]){{.colormap=XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone),.event_mask=PointerMotionMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask}});
	XSetWMProtocols(dpy,Wdo,(Atom[]){XInternAtom(dpy,"WM_DELETE_WINDOW",False)},1);
	XMapWindow(dpy,Wdo);
	glXMakeCurrent(dpy,Wdo,glXCreateContext(dpy,vi,0,GL_TRUE));
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
	rad[id=hand[32]]=145;
	cbts=hand[33];
	for(int i=0;i<4;i++){
		flag0[i]=i&1?hand[34+(i>>1)]>>4:hand[34+(i>>1)]&15;
		memcpy(flag[i],hand+36+i*3,3);
	}
	for(int i=0;i<8;i++)team[i]=i&1?hand[48+(i>>1)]>>4:hand[48+(i>>1)]&15;
	glOrtho(0,256,272,0,1,-1);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2,GL_SHORT,0,xyv);
	hud=glGenLists(1);
	mkhud();
	double fcx=xy[id][2]=-xy[id][0],fcy=xy[id][3]=-xy[id][1];
	for(;;){
		#ifdef GLX
		gettimeofday(&tvx,0);
		#else
		tvx=SDL_GetTicks();
		#endif
		bfp=buff;
		while(any(S)){
			int r=readch(),r5=r>>5;
			if(r==-1)return 0;
			switch(r&31){
			case(0)
				cbts^=1<<r5;
				if(!(cbts&1<<r5)){
					team[r5]=0;
					win[r5]=0;
					flag9(r5);
				}
			case(1 ... 6)mkwep(r&31,r5,0);
			case(7)
				B[Bs][0]=B[Bs+1][0]=r5;
				readx(bfp,4);
				memcpy(B[Bs]+1,bfp,2);
				memcpy(B[Bs+1]+1,bfp+2,2);
				*(uint16_t*)(B[Bs]+3)=*(uint16_t*)(B[Bs+1]+3)=0x2000;
				Bs+=2;
			case(8)readx(xy[r5],4);
			case(9)team[r5]=readch();
			case(10)
				rad[r5]=192;
				flag9(r5);
			case(11)
				r=readch();
				Wf(r&15,r>>4);
				mkhud();
			case(12)
				flag0[team[r5]-1]=9;
				flag[team[r5]-1][0]=xy[r5][0]&240|8;
				flag[team[r5]-1][1]=xy[r5][1]&240|8;
				flag[team[r5]-1][2]=xy[r5][0]>>4|xy[r5][1]&240;
			case(13)
				for(int j=0;j<4;j++)
					if(flag0[j]==r5+1){
						flag0[j]=9;
						flag[j][0]=flag[j][2]<<4|8;
						flag[j][1]=flag[j][2]&240|8;
						break;
					}
			case(14 ... 17)flag0[(r&31)-14]=r5+1;
			case(18)mode=readch();
			case(19)
				r=readch();
				tar[r&15]=r>>4;
			case(20)memset(tar,0,8);
			case(21)
				if(r5)win[r5-1]++;
				else{
					r=readch();
					for(int i=0;i<8;i++)win[i]+=!!(r&1<<i);
				}
			case(22)vir=r5;
			}
		}
		glCallList(hud);
		glBegin(GL_LINES);
		for(int i=0;i<4;i++)glVertex2i(w+(i>>1)<<4,i+1&2?272:256);
		if(sbo)
			cbts(i)
				if(tar[i]){
					glVertex2i(xy[i][2],xy[i][3]);
					glVertex2i(xy[tar[i]-1][0],xy[tar[i]-1][1]);
				}
		for(int i=0;i<8;i++)
			if(chg[ws[i]]){
				chg[ws[i]]--;
				for(int j=0;j<chg[ws[i]]>>4;j++)
					for(int k=0;k<2;k++)glVertex2i(i+k<<4,257+j);
				for(int j=0;j<2;j++)glVertex2i(i<<4|(j?chg[ws[i]]&15:0),257+(chg[ws[i]]>>4));
			}
		glEnd();
		cbts(i){
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
						if(i==id){
							#ifdef SDL
							time_t t=time(0)^tvx<<4;
							*(uint16_t*)(xy+id)^=SQR(t)^t;
							#else
							*(uint16_t*)(xy+id)^=tvx.tv_usec;
							#endif
						}
					}
				}else rad[i]++;
			}
		}
		for(int i=0;i<4;i++)
			if(flag0[i]){
				if(flag0[i]!=9){
					memcpy(flag[i],xy[flag0[i]-1],2);
					if(flag0[i]==id+1&&(flag[i][1]&240|flag[i][0]>>4)==flag[team[id]-1][2]){
						*bfp++=13;
						flag0[i]=9;
						flag[i][0]=flag[i][2]<<4|8;
						flag[i][1]=flag[i][2]&240|8;
					}
				}else(SQR(xy[id][0]-flag[i][0])+SQR(xy[id][1]-flag[i][1])<64&&rad[id]==64&&(team[id]!=i+1||(flag[i][1]&240|flag[i][0]>>4)!=flag[i][2])){
					for(int i=0;i<4;i++)
						if(flag0[i]==id+1)goto noj;
					*bfp++=14+i;
					flag0[i]=id+1;
				}
				noj:glColor3ubv(rgb[i]);
				glCirc(flag[i][0],flag[i][1],2);
				glCirc((flag[i][2]&15)<<4|8,flag[i][2]&240|8,3);
			}
		for(int i=0;i<Bs;){
			glColor3ubv(rgb[B[i][0]&127]);
			int b=(B[i][0]&128)==128,q=SQR(xy[id][0]-B[i][1])+SQR(xy[id][1]-B[i][2]);
			for(int j=b?B[i][4]-16:0;j<=B[i][3];j+=b?3:8)glCirc(B[i][1],B[i][2],j);
			glCirc(B[i][1],B[i][2],B[i][b?4:3]);
			if(B[i][3]==B[i][4]){
				memmove(B+i,B+i+1,(Bs-i)*5);
				Bs--;
				continue;
			}else(q<SQR(B[i][4])&&(b^(q<SQR(B[i][3]))))die(B[i][0]);
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
				if(A[i][5]++==255){killA:
					memmove(A+i,A+i+1,(As-i)*8);
					As--;
					continue;
				}else(W(xx>>4,yy>>4)){
					if(A[i][0]&224||A[i][5]++==255)goto killA;
					if(!((xx&15)<3?W((xx>>4)-1&15,yy>>4):(xx&15)>12?W((xx>>4)+1&15,yy>>4):1))A[i][0]|=128;
					if(!((yy&15)<3?W(xx>>4,(yy>>4)-1&15):(yy&15)>12?W(xx>>4,(yy>>4)+1&15):1))A[i][0]|=64;
					A[i][6]=xx;
					A[i][7]=yy;
				}else(SQR(xy[id][0]-xx)+SQR(xy[id][1]-yy)<64)die(A[i][0]);
				i++;
			}
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3,GL_UNSIGNED_BYTE,0,ahco);
			glDrawArrays(GL_POINTS,0,As);
			glDisableClientState(GL_COLOR_ARRAY);
		}
		int k5=0,t=team[id],wall=0;
		#ifdef AI
		uint_fast8_t wi[8];
		for(int i=0;i<8;i++)wi[ws[i]]=i;
		int mnv=130000,mni=id;
		mb=1;
		cbts(i)
			if(i!=id){
				int ln=SQR(xy[i][0]-xy[id][2])+SQR(xy[i][1]-xy[id][3]);
				if(ln<mnv){
					mnv=ln;
					mni=i;
				}
			}
		if(!chg[2]&&kda&&ksw&&SQR(xy[mni][0]-xy[id][0])+SQR(xy[mni][1]-xy[id][1])<3000)w=wi[2];
		else(!chg[3]&&mnv<4096)w=wi[3];
		else(!chg[5])w=wi[5];
		else(!chg[4]&&!W((xy[id][0]>>4)+(xy[id][2]>xy[id][0]?1:-1)&15,xy[id][1]>>4)&&!W(xy[id][0]>>4,(xy[id][1]>>4)+(xy[id][3]>xy[id][1]?1:-1)&15))w=wi[4];
		else(!chg[1])w=wi[1];
		else(!chg[7]){
			w=wi[7];
			if(mine){
				int mnv=1024;
				cbts(i)
					if(i!=id){
						int ln=SQR(xy[i][0]-B[39][0])+SQR(xy[i][1]-B[39][1]);
						if(ln<mnv)mnv=ln;
						ln=SQR(xy[i][0]-B[39][2])+SQR(xy[i][1]-B[39][3]);
						if(ln<mnv)mnv=ln;
					}
				if(mnv==1024)mb=0;
			}
		}
		int mv[4],mx=0;
		for(int i=-1;i<2;i++)
			for(int j=-1;j<2;j++){
				int mv=aimv(i,j,xy[id][0]+i,xy[id][1]+j,1);
				if(mv>mx){
					mx=mv;
					kda=i;
					ksw=j;
				}
			}
		if(mx<3&&!chg[6])w=wi[6];
		mx=xy[mni][0];
		my=xy[mni][1];
		#endif
		#ifdef GLX
		glXSwapBuffers(dpy,Wdo);
		XEvent ev;
		while(XPending(dpy)){
			KeySym ks;
			XNextEvent(dpy,&ev);
		#else
		SDL_GL_SwapWindow(dpy);
		SDL_Event ev;
		while(SDL_PollEvent(&ev)){
			SDL_Scancode ks;
		#endif
			switch(ev.type){
			#ifndef AI
			case(MotionNotify)
				mx=EV(motion.x);
				if(EV(motion.y)<256)my=EV(motion.y);
			case(KeyPress)case KeyRelease:{
				ks=KEYSYM;
				kda+=((ks==SDL_SCANCODE_D)-(ks==SDL_SCANCODE_A))*(ev.type==KeyPress?:-1);
				ksw+=((ks==SDL_SCANCODE_S)-(ks==SDL_SCANCODE_W))*(ev.type==KeyPress?:-1);
				if(kda>1)kda=1;
				else(kda<-1)kda=-1;
				if(ksw>1)ksw=1;
				else(ksw<-1)ksw=-1;
				int keq;
				if(ks==SDL_SCANCODE_B)mb=ev.type==KeyPress;
				else(ev.type==KeyRelease)break;
				else(ks==SDL_SCANCODE_P)sbo^=1;
				else(keq=(ks==SDL_SCANCODE_E)-(ks==SDL_SCANCODE_Q))w=w+keq&7;
				else(keq=(ks==SDL_SCANCODE_M)-(ks==SDL_SCANCODE_N)){
					uint8_t t=ws[w];
					ws[w]=ws[w+keq&7];
					ws[w+keq&7]=t;
					w=w+keq&7;
					mkhud();
				}else(ks>=SDL_SCANCODE_0&&ks<=SDL_SCANCODE_4)team[id]=ks-SDL_SCANCODE_0;
				else(ks==SDL_SCANCODE_5)k5=1;
				else(ks==SDL_SCANCODE_SPACE){
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
			#endif
			case(ClientMessage)return 0;
			}
		}
		if(t!=team[id])*bfp++=9|team[id]<<5;
		if(xy[id][2]!=mx)xy[id][2]=fcx+=(mx-xy[id][2])/sqrt(SQR(mx-xy[id][2])+SQR(my-xy[id][3]));
		if(xy[id][3]!=my)xy[id][3]=fcy+=(my-xy[id][3])/sqrt(SQR(mx-xy[id][2])+SQR(my-xy[id][3]));
		if(!W((xy[id][0]+kda&255)>>4,xy[id][1]>>4)||W(xy[id][0]>>4,xy[id][1]>>4))xy[id][0]+=kda;
		if(!W(xy[id][0]>>4,(xy[id][1]+ksw&255)>>4)||W(xy[id][0]>>4,xy[id][1]>>4))xy[id][1]+=ksw;
		if(!ws[w]&&mb&&!chg[0]&&!W(xy[id][2]>>4,xy[id][3]>>4)){
			chg[0]=255;
			memcpy(xy[id],xy[id]+2,2);
		}
		*bfp++=8;
		memcpy(bfp,xy+id,4);
		bfp+=4;
		if(k5&&team[id]&&(!flag0[team[id]-1]||(flag[team[id]-1][1]&240|flag[team[id]-1][0]>>4)!=flag[team[id]-1][2])){
			*bfp++=12;
			flag0[team[id]-1]=9;
			flag[team[id]-1][0]=xy[id][0]&240|8;
			flag[team[id]-1][1]=xy[id][1]&240|8;
			flag[team[id]-1][2]=xy[id][0]>>4|xy[id][1]&240;
		}
		if(mb&&ws[w]&&!chg[ws[w]]){
			if((*bfp=ws[w])<7)bfp+=mkwep(ws[w],id,1);
			else(mine=!mine){
				chg[7]=15;
				memcpy(B[39],xy[id],4);
			}else{
				chg[7]=180;
				B[Bs][0]=B[Bs+1][0]=id;
				*bfp++=7;
				memcpy(bfp,B[39],4);
				bfp+=4;
				memcpy(B[Bs]+1,B[39],2);
				memcpy(B[Bs+1]+1,B[39]+2,2);
				*(uint16_t*)(B[Bs]+3)=*(uint16_t*)(B[Bs+1]+3)=0x2000;
				Bs+=2;
			}
		}
		ship(buff,bfp-buff);
		#ifdef GLX
		gettimeofday(&tvy,0);
		if(tvy.tv_usec>tvx.tv_usec&&tvy.tv_usec-tvx.tv_usec<30000)usleep(33000-(tvy.tv_usec-tvx.tv_usec));
		#else
		tvy=SDL_GetTicks();
		if(tvy>tvx&&tvy-tvx<30)SDL_Delay(33-(tvy-tvx));
		#endif
	}
}