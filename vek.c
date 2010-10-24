#include <GL/glfw.h>
#include <math.h>
#include "net.h"
uint8_t rgb[8][3],x[8],y[8],cx[8],cy[8],cbts,id,w,dc[8],chg[8];
uint16_t port,W[16];
uint32_t mine=-1;
struct sockaddr_in servaddr;
uint8_t B[256][4],Bs,H[256][6],Hs,A[256][8],As,D[256][5],Ds;
int glDiam(int x,int y,int r){
	glBegin(GL_LINE_LOOP);
	glVertex2i(x+r,y);
	glVertex2i(x,y+r);
	glVertex2i(x-r,y);
	glVertex2i(x,y-r);
	glEnd();
}
int die(){
	dc[id]++;
	uint8_t xx=x[id];
	x[id]=-y[id];
	y[id]=-xx;
}
int quad(int x,int y){
	return x+y<16?(x<y?2:0):x<y?1:3;
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
		fprintf(stderr,"vek socket %d\n",errno);
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
	readx(W,32);
	glfwInit();
	if(!glfwOpenWindow(320,320,0,0,0,0,0,0,GLFW_WINDOW))return 1;
	glOrtho(0,320,320,0,1,-1);
	double t=0;
	for(;;){
		while(any()){
			uint8_t r=readch();
			switch(r&7){
			case(0)//CBTS
				cbts&=~(1<<(r>>5));
			case(1)//MOVE
				cx[r>>5]=readch();
				cy[r>>5]=readch();
				x[r>>5]=readch();
				y[r>>5]=readch();
			case(2)//WALL
				Wf(cx[r>>5]>>4,cy[r>>5]>>4);
			case(3)//BOMB
				B[Bs][0]=readch();
				B[Bs][1]=readch();
				B[Bs][2]=0;
				B[Bs][3]=r>>5;
				Bs++;
			case(4)//WAVE
				A[As][0]=x[r>>5];
				A[As][1]=y[r>>5];
				A[As][2]=cx[r>>5];
				A[As][3]=cy[r>>5];
				A[As][4]=0;
				A[As][5]=r>>5;
				A[As][6]=0;
				A[As][7]=0;
				As++;
			case(5)//SHOT
				H[Hs][0]=x[r>>5];
				H[Hs][1]=y[r>>5];
				H[Hs][2]=cx[r>>5];
				H[Hs][3]=cy[r>>5];
				H[Hs][4]=r>>5;
				H[Hs][5]=0;
				Hs++;
			case(6)//DOME
				D[Ds][0]=x[r>>5];
				D[Ds][1]=y[r>>5];
				D[Ds][2]=hypot(x[r>>5]-cx[r>>5],y[r>>5]-cy[r>>5]);
				D[Ds][3]=D[Ds][2]-16;
				D[Ds][4]=r>>5;
				Ds++;
			case(7)//MEET
				cbts|=1<<(r>>5);
				readx(rgb[r>>5],3);
			}
		}
		for(int i=0;i<8;i++)
			if(cbts&1<<i){
				glColor3ubv(rgb[i]);
				glBegin(GL_POINTS);
				glVertex2i(cx[i],cy[i]);
				glEnd();
				glDiam(x[i],y[i],4);
			}
		glColor3ubv(rgb[id]);
		for(int i=0;i<8;i++){
			if(chg[i])glRecti(256|i<<3,0,256|i+1<<3,chg[i]--);
		}
		glRecti(256|w<<3,256,256|w+1<<3,264);
		for(int i=0;i<16;i++)
			for(int j=0;j<16;j++)
				if(W(i,j)){
					glBegin(GL_LINE_LOOP);
					glVertex2i(j<<4,i<<4);
					glVertex2i(j+1<<4,i<<4);
					glVertex2i(j+1<<4,i+1<<4);
					glVertex2i(j<<4,i+1<<4);
					glEnd();
				}
		for(int i=0;i<As;){
			if(A[i][7]++==255){
				memmove(A+i,A+i+1,(As-i)*8);
				As--;
				continue;
			}
			A[i][6]++;
			uint8_t
				xx=A[i][0]+(A[i][2]-A[i][0])*A[i][6]*2/hypot(A[i][0]-A[i][2],A[i][1]-A[i][3]),
				yy=A[i][1]+(A[i][3]-A[i][1])*A[i][6]*2/hypot(A[i][0]-A[i][2],A[i][1]-A[i][3]);
			if(W(yy>>4,xx>>4)){
				if(quad(xx&15,yy&15)&2)A[i][2]=xx-(A[i][2]-xx);
				else A[i][3]=yy-(A[i][3]-yy);
				A[i][0]=xx;
				A[i][1]=yy;
				A[i][6]=1;
			}
			glColor3ubv(rgb[A[i][5]]);
			glDiam(xx,yy,2);
			if(A[i][5]!=id&&SQR(x[id]-xx)+SQR(y[id]-yy)<64)die();
			i++;
		}
		for(int i=0;i<Bs;){
			if(B[i][2]==80&&i<Bs){
				memmove(B+i,B+i+1,(Bs-i)*4);
				Bs--;
				continue;
			}
			glColor3ubv(rgb[B[i][3]]);
			for(int j=0;j<B[i][2];j+=8)glDiam(B[i][0],B[i][1],j);
			glDiam(B[i][0],B[i][1],B[i][2]);
			if(B[i][2]>8&&SQR(x[id]-B[i][0])+SQR(y[id]-B[i][1])<SQR(B[i][2]))die();
			B[i][2]++;
			i++;
		}
		for(int i=0;i<Ds;i++){
		}
		for(int i=0;i<Hs;){
			uint8_t
				xx=H[i][0]+(H[i][2]-H[i][0])*H[i][5]*2/hypot(H[i][0]-H[i][2],H[i][1]-H[i][3]),
				yy=H[i][1]+(H[i][3]-H[i][1])*H[i][5]*2/hypot(H[i][0]-H[i][2],H[i][1]-H[i][3]);
			if(W(yy>>4,xx>>4)||H[i][6]++==255){
				memmove(H+i,H+i+1,(Hs-i)*6);
				Hs--;
				continue;
			}
			glColor3ubv(rgb[H[i][4]]);
			glDiam(xx,yy,2);
			if(H[i][4]!=id&&SQR(x[id]-xx)+SQR(y[id]-yy)<64)die();
			i++;
		}
		glBegin(GL_LINE_STRIP);
		glVertex2i(256,0);
		glVertex2i(256,256);
		glVertex2i(0,256);
		glEnd();
		glfwSleep(1./30-(glfwGetTime()-t));
		t=glfwGetTime();
		uint8_t kq=glfwGetKey('Q'),ke=glfwGetKey('E');
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
		if(W(y[id]>>4,x[id]>>4))x[id]-=glfwGetKey(GLFW_KEY_RIGHT)-glfwGetKey(GLFW_KEY_LEFT);
		y[id]+=glfwGetKey(GLFW_KEY_DOWN)-glfwGetKey(GLFW_KEY_UP);
		if(W(y[id]>>4,x[id]>>4))y[id]-=glfwGetKey(GLFW_KEY_DOWN)-glfwGetKey(GLFW_KEY_UP);
		w=(w+(!ke&&glfwGetKey('E'))-(!kq&&glfwGetKey('Q')))&7;
		if(glfwGetKey('E')-glfwGetKey('Q'))fprintf(stderr,"%d",w);
		if(w==6&&!chg[6]&&glfwGetKey('Z')){
			x[id]=cx[id];
			y[id]=cy[id];
			chg[6]=255;
		}
		writech(1);
		writech(cx[id]);
		writech(cy[id]);
		writech(x[id]);
		writech(y[id]);
		if(w!=6&&!chg[w]&&glfwGetKey('Z')){
			switch(w){
			case(0)
				chg[0]=30;
				writech(4);
				H[Hs][0]=x[id];
				H[Hs][1]=y[id];
				H[Hs][2]=cx[id];
				H[Hs][3]=cy[id];
				H[Hs][4]=id;
				H[Hs][5]=0;
				Hs++;
			case(1)
				chg[1]=120;
				writech(7);
				B[Bs][0]=x[id];
				B[Bs][1]=y[id];
				B[Bs][2]=0;
				B[Bs][3]=id;
				Bs++;
			case(2)
				chg[2]=240;
				writech(8);
				B[Bs][0]=cx[id];
				B[Bs][1]=cy[id];
				B[Bs][2]=0;
				B[Bs][3]=id;
				Bs++;
			case(3)
				chg[3]=150;
				writech(2);
				D[Ds][0]=x[id];
				D[Ds][1]=y[id];
				D[Ds][2]=hypot(x[id]-cx[id],y[id]-cy[id]);
				D[Ds][3]=id;
				Ds++;
			case(4)
				chg[4]=90;
				writech(5);
				A[As][0]=x[id];
				A[As][1]=y[id];
				A[As][2]=cx[id];
				A[As][3]=cy[id];
				A[As][4]=0;
				A[As][5]=id;
				A[As][6]=0;
				A[As][7]=0;
				As++;
			case(5)
				chg[5]=15;
				writech(6);
				Wf(cy[id]>>4,cx[id]>>4);
			case(7)
				if(mine==-1){
					chg[7]=15;
					mine=cx[id]|cy[id]<<8|x[id]<<16|y[id]<<24;
				}else{
					chg[7]=240;
					writech(3);
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
	}
}