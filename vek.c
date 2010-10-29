#ifdef GLX
#include <GL/glx.h>
#include <time.h>
#else
#include <GL/glfw.h>
#endif
#include <math.h>
#include "net.h"
uint8_t rgb[8][3],x[8],y[8],cx[8],cy[8],cbts=0,id,w,chg[8];
uint16_t port,W[16];
uint32_t mine=-1;
int mx,my,mb,ka,kw,kd,ks;
FILE*rnd;
GLuint hud;
struct sockaddr_in addr;
uint8_t B[256][4],Bs,H[256][6],Hs,D[256][5],Ds;
void glCirc(int x,int y,int r){
	int r2=r*r,r12=r*M_SQRT1_2;
	for(uint8_t xc=0;xc<=r12;xc++){
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
		x[id]=fgetc(rnd);
		y[id]=fgetc(rnd);
		if(!W(x[id]>>4,y[id]>>4))return;
	}
}
void axit(){
	writech(9);
	ship();
	close(S);
}
int main(int argc,char**argv){
	atexit(axit);
	rnd=fopen("/dev/urandom","r");
	if(argc<2){
		puts("vektorael ip:port");
		return 1;
	}
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	char*szPort=strchr(argv[1],':');
	if(szPort){
		*szPort++=0;
		port=strtol(szPort,0,0);
	}else port=2000;
	if((S=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"vek socket %d\n",errno);
		return 1;
	}
	addr.sin_port=htons(port);
	if(inet_aton(argv[1],&addr.sin_addr)<=0){
		fprintf(stderr,"vek ip %d\n",errno);
		return 1;
	}
	if(connect(S,(struct sockaddr*)&addr,sizeof(addr))<0){
		fprintf(stderr,"vek connect %d\n",errno);
		return 1;
	}
	uint8_t rgb2[3]={64|fgetc(rnd),64|fgetc(rnd),64|fgetc(rnd)};
	writex(rgb2,3);
	ship();
	id=readch();
	cbts=readch();
	for(int i=0;i<8;i++)
		if(cbts&1<<i&&i!=id)readx(rgb[i],3);
	readx(W,32);
	die();
	float fcx=cx[id]=-x[id],fcy=cy[id]=-y[id];
	memcpy(rgb[id],rgb2,3);
	#ifdef GLX
	Display*dpy=XOpenDisplay(0);
	XVisualInfo*vi=glXChooseVisual(dpy,DefaultScreen(dpy),(int[]){GLX_RGBA,GLX_DOUBLEBUFFER,None});
	Window win=XCreateWindow(dpy,RootWindow(dpy,vi->screen),0,0,256,274,0,vi->depth,InputOutput,vi->visual,CWColormap|CWEventMask,(XSetWindowAttributes[]){{.colormap=XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone),.event_mask=PointerMotionMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask}});
	XMapWindow(dpy,win);
	GLXContext ctx=glXCreateContext(dpy,vi,0,GL_TRUE);
	glXMakeCurrent(dpy,win,ctx);
	#else
	glfwInit();
	if(!glfwOpenWindow(256,274,0,0,0,0,0,0,GLFW_WINDOW))return 1;
	#endif
	glOrtho(0,256,274,0,1,-1);
	glNewList(hud=glGenLists(1),GL_COMPILE);
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
	glEndList();
	for(;;){
		while(any()){
			uint8_t r=readch();
			switch(r&15){
			case(0)//SHOT
				H[Hs][0]=x[r>>5];
				H[Hs][1]=y[r>>5];
				H[Hs][2]=cx[r>>5];
				H[Hs][3]=cy[r>>5];
				H[Hs][4]=r>>5;
				H[Hs][5]=0;
				Hs++;
			case(1)//BOMB
				B[Bs][0]=x[r>>5];
				B[Bs][1]=y[r>>5];
				B[Bs][2]=0;
				B[Bs][3]=r>>5;
				Bs++;
			case(2)//NUKE
				B[Bs][0]=cx[r>>5];
				B[Bs][1]=cy[r>>5];
				B[Bs][2]=0;
				B[Bs][3]=r>>5;
				Bs++;
			case(3)//DOME
				D[Ds][0]=x[r>>5];
				D[Ds][1]=y[r>>5];
				D[Ds][2]=hypot(x[r>>5]-cx[r>>5],y[r>>5]-cy[r>>5]);
				D[Ds][3]=D[Ds][2]-16;
				D[Ds][4]=r>>5;
				Ds++;
			case(5)//WALL
				Wf(cx[r>>5]>>4,cy[r>>5]>>4);
			case(6)//MEET
				cbts|=1<<(r>>5);
				readx(rgb[r>>5],3);
			case(7)//MINE
				B[Bs][0]=readch();
				B[Bs][1]=readch();
				B[Bs][2]=0;
				B[Bs][3]=r>>5;
				B[Bs+1][0]=readch();
				B[Bs+1][1]=readch();
				B[Bs+1][2]=0;
				B[Bs+1][3]=r>>5;
				Bs+=2;
			case(8)//MOVE
				cx[r>>5]=readch();
				cy[r>>5]=readch();
				x[r>>5]=readch();
				y[r>>5]=readch();
			case(9)//CBTS
				cbts&=~(1<<(r>>5));
			}
		}
		glCallList(hud);
		glBegin(GL_LINE_STRIP);
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
				glVertex2i(cx[i],cy[i]);
				glCirc(x[i],y[i],4);
			}
		for(int i=0;i<Bs;){
			if(B[i][2]==80&&i<Bs){
				memmove(B+i,B+i+1,(Bs-i)*4);
				Bs--;
				continue;
			}
			glColor3ubv(rgb[B[i][3]]);
			for(int j=0;j<B[i][2];j+=8)glCirc(B[i][0],B[i][1],j);
			glCirc(B[i][0],B[i][1],B[i][2]);
			if(B[i][2]>8&&SQR(x[id]-B[i][0])+SQR(y[id]-B[i][1])<SQR(B[i][2]))die();
			B[i][2]++;
			i++;
		}
		for(int i=0;i<Ds;){
			glColor3ubv(rgb[D[i][4]]);
			for(int j=D[i][2]-16;j<=D[i][3];j+=2)glCirc(D[i][0],D[i][1],j);
			glCirc(D[i][0],D[i][1],D[i][2]);
			D[i][3]+=2;
			if(D[i][3]==D[i][2]){
				if(SQR(x[id]-D[i][0])+SQR(y[id]-D[i][1])<SQR(D[i][2])&&SQR(x[id]-D[i][0])+SQR(y[id]-D[i][1])>SQR(D[i][2]-16))die();
				memmove(D+i,D+i+1,(Hs-i)*5);
				Ds--;
				continue;
			}
			i++;
		}
		for(int i=0;i<Hs;){
			uint8_t
				xx=H[i][0]+(H[i][2]-H[i][0])*H[i][5]*2/hypot(H[i][0]-H[i][2],H[i][1]-H[i][3]),
				yy=H[i][1]+(H[i][3]-H[i][1])*H[i][5]*2/hypot(H[i][0]-H[i][2],H[i][1]-H[i][3]);
			if(W(xx>>4,yy>>4)||H[i][5]++==255){
				memmove(H+i,H+i+1,(Hs-i)*6);
				Hs--;
				continue;
			}
			glColor3ubv(rgb[H[i][4]]);
			glVertex2i(xx,yy);
			if(H[i][4]!=id&&SQR(x[id]-xx)+SQR(y[id]-yy)<64)die();
			i++;
		}
		glEnd();
		#if GLX
		XEvent ev;
		while(XPending(dpy)){
			XNextEvent(dpy,&ev);
			switch(ev.type){
			KeySym keysym;
			char buff;
			case(KeyPress)
				if(XLookupString((XKeyEvent*)&ev,&buff,1,&keysym,NULL)==1)
					switch(buff){
					case('d')kd=1;
					case('a')ka=1;
					case('s')ks=1;
					case('w')kw=1;
					case(' ')
						FILE*lv=fopen("lv","wb");
						if(lv){
							fwrite(W,16,2,lv);
							fclose(lv);
						}else puts("Failed to open");
					case(27)return 0;
					}
			case(KeyRelease)
				if(XLookupString((XKeyEvent*)&ev,&buff,1,&keysym,NULL)==1)
					switch(buff){
					case('d')kd=0;
					case('a')ka=0;
					case('s')ks=0;
					case('w')kw=0;
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
		glXSwapBuffers(dpy,win);
		#else
		printf("%f\n",glfwGetTime());
		glfwSwapBuffers();
		if(glfwGetKey(GLFW_KEY_ESC)||!glfwGetWindowParam(GLFW_OPENED))return 0;
		glfwGetMousePos(&mx,&my);
		mb=glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
		kd=glfwGetKey('D');
		ka=glfwGetKey('A');
		ks=glfwGetKey('S');
		kw=glfwGetKey('W');
		w=w+glfwGetMouseWheel()&7;
		glfwSetMouseWheel(0);
		if(glfwGetKey(GLFW_KEY_SPACE)){
			FILE*lv=fopen("lv","wb");
			if(lv){
				fwrite(W,16,2,lv);
				fclose(lv);
			}else puts("Failed to open");
		}
		#endif
		glClear(GL_COLOR_BUFFER_BIT);
		if(cx[id]!=mx)cx[id]=fcx=fmin(fmax(fcx+(mx-cx[id])/hypot(mx-cx[id],my-cy[id]),0),255);
		if(cy[id]!=my)cy[id]=fcy=fmin(fmax(fcy+(my-cy[id])/hypot(mx-cx[id],my-cy[id]),0),255);
		x[id]+=kd-ka;
		if(W(x[id]>>4,y[id]>>4))x[id]-=kd-ka;
		y[id]+=ks-kw;
		if(W(x[id]>>4,y[id]>>4))y[id]-=ks-kw;
		if(w==6&&!chg[6]&&!W(cx[id]>>4,cy[id]>>4)&&mb){
			chg[6]=255;
			x[id]=cx[id];
			y[id]=cy[id];
		}
		writech(8);
		writech(cx[id]);
		writech(cy[id]);
		writech(x[id]);
		writech(y[id]);
		if(w!=6&&!chg[w]&&mb){
			writech(w);
			switch(w){
			case(0)
				chg[0]=30;
				H[Hs][0]=x[id];
				H[Hs][1]=y[id];
				H[Hs][2]=cx[id];
				H[Hs][3]=cy[id];
				H[Hs][4]=id;
				H[Hs][5]=0;
				Hs++;
			case(1)
				chg[1]=120;
				B[Bs][0]=x[id];
				B[Bs][1]=y[id];
				B[Bs][2]=0;
				B[Bs][3]=id;
				Bs++;
			case(2)
				chg[2]=240;
				B[Bs][0]=cx[id];
				B[Bs][1]=cy[id];
				B[Bs][2]=0;
				B[Bs][3]=id;
				Bs++;
			case(3)
				chg[3]=150;
				D[Ds][0]=x[id];
				D[Ds][1]=y[id];
				D[Ds][2]=hypot(x[id]-cx[id],y[id]-cy[id]);
				D[Ds][3]=D[Ds][2]-16;
				D[Ds][4]=id;
				Ds++;
			case(5)
				chg[5]=15;
				Wf(cx[id]>>4,cy[id]>>4);
			case(7)
				if(mine==-1){
					chg[7]=15;
					mine=cx[id]|cy[id]<<8|x[id]<<16|y[id]<<24;
					blen=0;
				}else{
					chg[7]=240;
					writech(mine);
					writech(mine>>8);
					writech(mine>>16);
					writech(mine>>24);
					B[Bs][0]=mine;
					B[Bs][1]=mine>>8;
					B[Bs][2]=0;
					B[Bs][3]=id;
					B[Bs+1][0]=mine>>16;
					B[Bs+1][1]=mine>>24;
					B[Bs+1][2]=0;
					B[Bs+1][3]=id;
					Bs+=2;
					mine=-1;
				}
			}
		}
		ship();
		#ifdef GLX
		usleep(30000);
		#else
		glfwSleep(1./30-glfwGetTime());
		glfwSetTime(0);
		#endif
	}
}