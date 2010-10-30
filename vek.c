#ifdef GLX
#include <GL/glx.h>
#include <sys/time.h>
struct timeval tvx,tvy;
#else
#include <GL/glfw.h>
#endif
#include "net.h"
#include "math.h"
uint8_t rgb[8][3],xy[8][4],cbts,id,w,chg[8],buff[10]={8},mine,B[256][5],Bs,H[256][6],Hs,D[256][5],Ds;
uint16_t port,W[16];
int mx,my,mb,kda,ksw;
FILE*rnd;
GLuint hud;
struct sockaddr_in addr;
void glCirc(int x,int y,int r){
	int r2=r*r,r12=r*M_SQRT1_2;
	for(int xc=0;xc<=r12;xc++){
		if(xc*xc+r*r>=r2)r--;
		glVertex2i(x+xc,y+r);
		glVertex2i(x+xc,y-r);
		glVertex2i(x-xc,y+r);
		glVertex2i(x-xc,y-r);
		glVertex2i(x+r,y+xc);
		glVertex2i(x+r,y-xc);
		glVertex2i(x-r,y+xc);
		glVertex2i(x-r,y-xc);
	}
}
void die(){
	for(int i=0;i<256;i++){
		fread(xy+id,2,1,rnd);
		if(!W(xy[id][0]>>4,xy[id][1]>>4))return;
	}
}
void axit(){
	char b=9;
	while(write(S,&b,1)!=1);
	close(S);
}
int main(int argc,char**argv){
	atexit(axit);
	rnd=fopen("/dev/urandom","r");
	if(argc<2){
		puts("vektorael ip:port");
		return 1;
	}
	addr.sin_family=AF_INET;
	char*szPort=strchr(argv[1],':');
	if(szPort){
		*szPort++=0;
		port=strtol(szPort,0,0);
	}else port=2000;
	if((S=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"socket %d\n",errno);
		return 1;
	}
	addr.sin_port=htons(port);
	if(inet_aton(argv[1],&addr.sin_addr)<=0){
		fprintf(stderr,"ip %d\n",errno);
		return 1;
	}
	if(connect(S,(struct sockaddr*)&addr,sizeof(addr))<0){
		fprintf(stderr,"connect %d\n",errno);
		return 1;
	}
	uint8_t rgb2[3]={64|fgetc(rnd),64|fgetc(rnd),64|fgetc(rnd)};
	ship(rgb2,3);
	id=readch();
	cbts=readch();
	for(int i=0;i<8;i++)
		if(cbts&1<<i&&i!=id)readx(rgb[i],3);
	readx(W,32);
	die();
	float fcx=xy[id][2]=-xy[id][0],fcy=xy[id][3]=-xy[id][1];
	memcpy(rgb[id],rgb2,3);
	#ifdef GLX
	Display*dpy=XOpenDisplay(0);
	XVisualInfo*vi=glXChooseVisual(dpy,DefaultScreen(dpy),(int[]){GLX_RGBA,GLX_DOUBLEBUFFER,None});
	Window win=XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,256,274,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,(XSetWindowAttributes[]){{.colormap=XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone),.event_mask=PointerMotionMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask}});
	XMapWindow(dpy,win);
	glXMakeCurrent(dpy,win,glXCreateContext(dpy,vi,0,GL_TRUE));
	gettimeofday(&tvx,0);
	#else
	glfwInit();
	if(!glfwOpenWindow(256,274,0,0,0,0,0,0,GLFW_WINDOW))return 1;
	#endif
	glOrtho(0,256,274,0,1,-1);
	glNewList(hud=glGenLists(1),GL_COMPILE);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3ubv(rgb[id]);
	glBegin(GL_LINE_STRIP);
	glVertex2i(97,257);
	glVertex2i(113,257);
	glVertex2i(113,273);
	glVertex2i(100,273);
	glVertex2i(100,260);
	glVertex2i(110,260);
	glVertex2i(110,270);
	glVertex2i(103,270);
	glVertex2i(103,263);
	glVertex2i(107,263);
	glVertex2i(107,266);
	glEnd();
	glBegin(GL_LINES);
	glVertex2i(0,256);
	glVertex2i(256,256);
	glVertex2i(1,257);
	glVertex2i(15,271);
	glVertex2i(40,258);
	glVertex2i(40,265);
	glVertex2i(81,257);
	glVertex2i(95,271);
	glVertex2i(95,257);
	glVertex2i(81,271);
	glEnd();
	glBegin(GL_POINTS);
	glCirc(23,265,6);
	glCirc(23,265,4);
	glCirc(39,265,5);
	glCirc(55,265,7);
	glCirc(118,263,4);
	glCirc(122,267,4);
	glVertex2i(55,265);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glEndList();
	for(;;){
		while(any(S)){
			uint8_t r=readch();
			switch(r&15){
			case(0)//SHOT
				H[Hs][0]=r>>5;
				memcpy(H[Hs]+1,xy[r>>5],4);
				H[Hs][5]=0;
				Hs++;
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
			case(5)//WALL
				Wf(xy[r>>5][2]>>4,xy[r>>5][3]>>4);
			case(6)//MEET
				cbts|=1<<(r>>5);
				readx(rgb[r>>5],3);
			case(7)//MINE
				B[Bs][0]=B[Bs+1][0]=r>>5;
				readx(B[Bs]+1,2);
				readx(B[Bs+1]+1,2);
				B[Bs][3]=B[Bs+1][3]=0;
				B[Bs][4]=B[Bs+1][4]=32;
				Bs+=2;
			case(8)//MOVE
				readx(xy[r>>5],4);
			case(9)//CBTS
				cbts&=~(1<<(r>>5));
			}
		}
		glCallList(hud);
		glVertex2i(w<<4,256);
		glVertex2i(w<<4,273);
		glVertex2i(w+1<<4,273);
		glVertex2i(w+1<<4,256);
		glEnd();
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
		glBegin(GL_POINTS);
		for(int i=0;i<8;i++)
			if(chg[i]){
				chg[i]--;
				for(int j=0;j<chg[i];j++)glVertex2i(i<<4|j&15,258+(j>>4));
			}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				glColor3ubv(rgb[i]);
				glVertex2i(xy[i][2],xy[i][3]);
				glCirc(xy[i][0],xy[i][1],4);
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
			if(H[i][0]!=id&&SQR(xy[id][0]-xx)+SQR(xy[id][1]-yy)<64)die();
			i++;
		}
		glEnd();
		int k_=0;
		#ifdef GLX
		glXSwapBuffers(dpy,win);
		XEvent ev;
		while(XPending(dpy)){
			XNextEvent(dpy,&ev);
			switch(ev.type){
			case(KeyPress)
				switch(XKeycodeToKeysym(dpy,ev.xkey.keycode,0)){
				case(XK_d)kda++;
				case(XK_a)kda--;
				case(XK_s)ksw++;
				case(XK_w)ksw--;
				case(XK_space)k_=1;
				case(XK_Escape)return 0;
				}
			case(KeyRelease)
				switch(XKeycodeToKeysym(dpy,ev.xkey.keycode,0)){
				case(XK_d)kda--;
				case(XK_a)kda++;
				case(XK_s)ksw--;
				case(XK_w)ksw++;
				}
			case(ButtonPress)
				if(ev.xbutton.button<4)mb=1;
				else w=w+(ev.xbutton.button==4)-(ev.xbutton.button==5)&7;
			case(ButtonRelease)mb=0;
			case(MotionNotify)
				mx=ev.xmotion.x;
				my=ev.xmotion.y;
			}
		}
		#else
		glfwSwapBuffers();
		if(glfwGetKey(GLFW_KEY_ESC)||!glfwGetWindowParam(GLFW_OPENED))return 0;
		glfwGetMousePos(&mx,&my);
		if(mb=glfwGetMouseWheel()){
			w=w+mb&7;
			glfwSetMouseWheel(0);
		}
		mb=glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
		kda=glfwGetKey('D')-glfwGetKey('A');
		ksw=glfwGetKey('S')-glfwGetKey('W');
		k_=glfwGetKey(GLFW_KEY_SPACE);
		#endif
		if(k_){
			FILE*lv=fopen("lv","wb");
			if(lv){
				fwrite(W,16,2,lv);
				fclose(lv);
			}
		}
		if(xy[id][2]!=mx)xy[id][2]=fcx=fmin(fmax(fcx+(mx-xy[id][2])/hypot(mx-xy[id][2],my-xy[id][3]),0),255);
		if(xy[id][3]!=my)xy[id][3]=fcy=fmin(fmax(fcy+(my-xy[id][3])/hypot(mx-xy[id][2],my-xy[id][3]),0),255);
		xy[id][0]+=kda;
		if(W(xy[id][0]>>4,xy[id][1]>>4))xy[id][0]-=kda;
		xy[id][1]+=ksw;
		if(W(xy[id][0]>>4,xy[id][1]>>4))xy[id][1]-=ksw;
		if(w==6&&!chg[6]&&!W(xy[id][2]>>4,xy[id][3]>>4)&&mb){
			chg[6]=255;
			xy[id][0]=xy[id][2];
			xy[id][1]=xy[id][3];
		}
		memcpy(buff+1,xy+id,4);
		uint8_t l=5;
		if(w!=6&&!chg[w]&&mb){
			switch(buff[l++]=w){
			case(0)
				chg[0]=30;
				H[Hs][0]=id;
				memcpy(H[Hs]+1,xy[id],4);
				H[Hs][5]=0;
				Hs++;
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
			case(5)
				chg[5]=15;
				Wf(xy[id][2]>>4,xy[id][3]>>4);
			case(7)
				if(mine=!mine){
					chg[7]=15;
					l=5;
					memcpy(buff+6,xy[id],4);
				}else{
					chg[7]=240;
					l=10;
					B[Bs][0]=B[Bs+1][0]=id;
					memcpy(B[Bs]+1,buff+6,2);
					memcpy(B[Bs+1]+1,buff+8,2);
					B[Bs][3]=B[Bs+1][3]=0;
					B[Bs][4]=B[Bs+1][4]=32;
					Bs+=2;
				}
			}
		}
		ship(buff,l);
		#ifdef GLX
		gettimeofday(&tvy,0);
		fprintf(stderr,"%d\n",tvy.tv_usec-tvx.tv_usec);
		if(tvy.tv_usec>tvx.tv_usec)usleep(33333-(tvy.tv_usec-tvx.tv_usec));
		gettimeofday(&tvx,0);
		#else
		printf("%f\n",glfwGetTime()*1000000);
		glfwSleep(1./30-glfwGetTime());
		glfwSetTime(0);
		#endif
	}
}