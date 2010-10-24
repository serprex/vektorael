#include <GL/glfw.h>
#include <math.h>
#include "net.h"
uint8_t rgb[8][3],x[8],y[8],cx[8],cy[8],cbts,id,w,W[16][16];
uint16_t port,chg[8];
uint32_t mine=-1;
struct sockaddr_in servaddr;
uint8_t B[256][3],Bs,H[256][4],Hs,A[256][5],As,D[256][3],Ds;
int glDiam(int x,int y,int r){
	glBegin(GL_LINE_LOOP);
	glVertex2i(x+r,y);
	glVertex2i(x,y+r);
	glVertex2i(x-r,y);
	glVertex2i(x,y-r);
	glEnd();
}
int main(int argc,char**argv){
	if(argc<2){
		puts("vektorael ip:port");
		return 1;
	}
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	char*szPort=strchr(argv[1],':');
	if(szPort){
		*szPort++=0;
		port=strtol(szPort,0,0);
	}else port=2000;
	if((S=socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr,"vek socket\n",errno);
		return 1;
	}
	servaddr.sin_port=htons(port);
	if(inet_aton(argv[1],&servaddr.sin_addr)<=0){
		fprintf(stderr,"vek ip %d\n",errno);
		return 1;
	}
	if(connect(S,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		fprintf(stderr,"vek connect %d\n",errno);
		return 1;
	}
	uint8_t rgb2[3]={80,160,240};
	writex(rgb2,3);
	ship();
	id=readch();
	memcpy(rgb[id],rgb2,3);
	cbts=readch();
	for(int i=0;i<8;i++)
		if(cbts&1<<i&&i!=id){
			readx(rgb[i],3);
		}
	glfwInit();
	if(!glfwOpenWindow(320,320,0,0,0,0,0,0,GLFW_WINDOW))return 1;
	glOrtho(0,320,320,0,1,-1);
	double t=0;
	for(;;){
		while(any()){
			uint8_t r=readch();
			switch(r&7){
			uint8_t z;
			case(0)//CBTS
				cbts=r>>5;
			case(1)//MOVE
				cx[r>>5]=readch();
				cy[r>>5]=readch();
				x[r>>5]=readch();
				y[r>>5]=readch();
			case(2)//WALL
				z=readch();
				W[z>>4][z&15]=!W[z>>4][z&15];
			case(3)//BOMB
				B[Bs][0]=readch();
				B[Bs][1]=readch();
				B[Bs][2]=0;
				Bs++;
			case(4)//WAVE
				A[As][0]=x[r>>5];
				A[As][1]=y[r>>5];
				A[As][2]=cx[r>>5];
				A[As][3]=cy[r>>5];
				A[As][4]=0;
				As++;
			case(5)//SHOT
				H[Hs][0]=x[r>>5];
				H[Hs][1]=y[r>>5];
				H[Hs][2]=cx[r>>5];
				H[Hs][3]=cy[r>>5];
				Hs++;
			case(6)//DOME
				D[Ds][0]=x[r>>5];
				D[Ds][1]=y[r>>5];
				D[Ds][2]=hypot(x[r>>5]-cx[r>>5],y[r>>5]-cy[r>>5]);
				Ds++;
			case(7)//MEET
				cbts|=1<<(r>>5);
				readx(rgb[r>>5],3);
			}
		}
		for(int i=0;i<8;i++)
			if(chg[i])chg[i]--;
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				glColor3ubv(rgb[i]);
				glBegin(GL_POINTS);
				glVertex2i(cx[i],cy[i]);
				glEnd();
				glDiam(x[i],y[i],3);
			}
		glColor3ubv(rgb[id]);
		for(int i=0;i<16;i++)
			for(int j=0;j<16;j++)
				if(W[i][j]){
					glBegin(GL_LINE_LOOP);
					glVertex2i(j<<4,i<<4);
					glVertex2i(j+1<<4,i<<4);
					glVertex2i(j+1<<4,i+1<<4);
					glVertex2i(j<<4,i+1<<4);
					glEnd();
				}
		glBegin(GL_LINE_STRIP);
		glVertex2i(255,0);
		glVertex2i(255,255);
		glVertex2i(0,255);
		glEnd();
		glfwSleep(1./30-(glfwGetTime()-t));
		t=glfwGetTime();
		uint8_t kw=glfwGetKey('W'),ke=glfwGetKey('E');
		glfwSwapBuffers();
		if(glfwGetKey(GLFW_KEY_ESC)||!glfwGetWindowParam(GLFW_OPENED)){
			writech(0);
			ship();
			close(S);
			return 0;
		}
		glClear(GL_COLOR_BUFFER_BIT);
		cx[id]+=glfwGetKey('D')-glfwGetKey('A');
		cy[id]+=glfwGetKey('S')-glfwGetKey('W');
		x[id]+=glfwGetKey(GLFW_KEY_RIGHT)-glfwGetKey(GLFW_KEY_LEFT);
		y[id]+=glfwGetKey(GLFW_KEY_DOWN)-glfwGetKey(GLFW_KEY_UP);
		w=(w+(!ke&&glfwGetKey('E'))-(!kw&&glfwGetKey('Q')))&7;
		if(glfwGetKey('E')-glfwGetKey('Q'))fprintf(stderr,"%d",w);
		if(w==6&&!chg[6]&&glfwGetKey(GLFW_KEY_SPACE)){
			x[id]=cx[id];
			y[id]=cy[id];
			chg[6]=300;
		}
		writech(cx[id]|1);
		writech(cy[id]);
		writech(x[id]);
		writech(y[id]);
		if(w!=6&&!chg[w]&&glfwGetKey(GLFW_KEY_SPACE)){
			switch(w){
			case(0)
				chg[0]=30;
				writech(14);
				H[Hs][0]=x[id];
				H[Hs][1]=y[id];
				H[Hs][2]=cx[id];
				H[Hs][3]=cy[id];
				Hs++;
			case(1)
				chg[1]=180;
				writech(26);
				B[Bs][0]=x[id];
				B[Bs][1]=y[id];
				B[Bs][2]=0;
				Bs++;
			case(2)
				chg[2]=240;
				writech(30);
				B[Bs][0]=cx[id];
				B[Bs][1]=cy[id];
				B[Bs][2]=0;
				Bs++;
			case(3)
				chg[3]=150;
				writech(6);
			case(4)
				chg[4]=90;
				writech(18);
				A[As][0]=x[id];
				A[As][1]=y[id];
				A[As][2]=cx[id];
				A[As][3]=cy[id];
				A[As][4]=0;
				As++;
			case(5)
				chg[5]=300;
				writech(22);
				W[cy[id]>>4][cx[id]>>4]=!W[cy[id]>>4][cx[id]>>4];
			case(7)
				if(mine==65535){
					mine=cx[id]|cy[id]<<8|x[id]<<16|y[id]<<24;
				}else{
					chg[7]=240;
					writech(10);
					writech(mine);
					writech(mine>>8);
					writech(mine>>16);
					writech(mine>>24);
					B[Bs][0]=mine;
					B[Bs][1]=mine>>8;
					B[Bs][2]=0;
					B[Bs+1][0]=mine>>16;
					B[Bs+1][1]=mine>>24;
					B[Bs+1][2]=0;
					Bs+=2;
				}
			}
		}
		ship();
	}
}