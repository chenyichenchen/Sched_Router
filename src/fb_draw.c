#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "fb_draw.h"
#include "readjpeg.h"

int fbfd = 0;
char *fbp = 0;
int fw = 0,fh = 0,fc = 0,fs = 0;
char *fbuf = 0;
char *fbg = 0;

color C_R = {255,0,0};
color C_G = {0,255,0};
color C_B = {0,0,255};
color C_Y = {222,125,44};
color C_K = {0,0,0};
color C_W = {255,255,255};

char pixelmap[55]={0x3E,0x41,0x41,0x41,0x3E, //0
                   0x00,0x00,0x21,0x7F,0x01, //1
                   0x27,0x45,0x45,0x45,0x39, //2
                   0x22,0x49,0x49,0x49,0x36, //3
                   0x0C,0x14,0x24,0x7F,0x04, //4
                   0x72,0x51,0x51,0x51,0x4E, //5
                   0x3E,0x49,0x49,0x49,0x26, //6
                   0x40,0x40,0x40,0x4F,0x70, //7
                   0x36,0x49,0x49,0x49,0x36, //8
                   0x32,0x49,0x49,0x49,0x3E, //9
                   0x22,0x04,0x08,0x10,0x22  //%
                  };
color rand_color(){
  color c;
  c.R = rand()%255;
  c.G = rand()%255;
  c.B = rand()%255;
  return c;
}
int bg_init(){
  int x,y,src,dst;
  int width,height,components;
  readjpeg("config/map.jpg",fbuf,&width,&height,&components);
  for(y=0;y<height;y++)
    for(x=0;x<width;x++){
      src = (y*width+x)*components;
      dst = (y*fw+x)*fc;
      fbg[dst+0] = fbuf[src+2]; 
      fbg[dst+1] = fbuf[src+1]; 
      fbg[dst+2] = fbuf[src+0]; 
    }
  memset(fbuf,0,fs);
  return 0;
}
int fb_init(){
  struct fb_var_screeninfo vinfo;
  fbfd = open("/dev/fb0", O_RDWR);
  ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
  fw = vinfo.xres_virtual; fh = vinfo.yres_virtual; fc = vinfo.bits_per_pixel/8;
  fs = fw*fh*fc;
  fbuf = (char *)malloc(fs);
  memset(fbuf,0,fs);
  fbg = (char *)malloc(fs);
  memset(fbg,0,fs);
  bg_init();
  fbp = (char *)mmap(0,fs,PROT_READ|PROT_WRITE, MAP_SHARED,fbfd,0);
  srand(time(NULL));
  return 0;
}
int fb_background(){
  memcpy(fbuf,fbg,fs);
}
int fb_show(){
  memcpy(fbp,fbuf,fs);
  return 0;
}
int fb_fini(){
  munmap(fbp,fs);
  free(fbg);
  free(fbuf);
  close(fbfd);
  return 0;
}
void draw_clear(){
  memset(fbuf,0,fs);
  memcpy(fbp,fbuf,fs);
}
void draw_pic(char *pbuf, int width, int height, int channel){
  int x,y,src,dst;
  for(y=0;y<height;y++)
    for(x=0;x<width;x++){
      src = (y*width+x)*channel;
      dst = (y*fw+x)*fc;
      fbuf[dst+0] = pbuf[src+2]; 
      fbuf[dst+1] = pbuf[src+1]; 
      fbuf[dst+2] = pbuf[src+0]; 
    }
}
void draw_point(char *buf,point p, color c){
  int pos = p.y*fw+p.x;
  buf[pos*fc+0] = c.B;
  buf[pos*fc+1] = c.G;
  buf[pos*fc+2] = c.R;
}
void draw_line(char *buf,point p1, point p2, color c){
  point p;
  int x,y;
  int min,max;
  float ratio;
  if(p1.x == p2.x){
    if(p1.y<p2.y){
      for(y=p1.y;y<=p2.y;y++){
        p.x = p1.x;
        p.y = y;
        draw_point(buf,p,c);
      }
      return;
    }else if(p1.y>p2.y){
      for(y=p2.y;y<=p1.y;y++){
        p.x = p1.x;
        p.y = y;
        draw_point(buf,p,c);
      }
      return;
    }else{
      draw_point(buf,p1,c);
      return;
    }
  }else if(p1.y == p2.y){
    if(p1.x<p2.x){
      for(x=p1.x;x<=p2.x;x++){
        p.x = x;
        p.y = p1.y;
        draw_point(buf,p,c);
      }
      return;
    }else if(p1.x>p2.x){
      for(x=p2.x;x<=p1.x;x++){
        p.x = x;
        p.y = p1.y;
        draw_point(buf,p,c);
      }
      return;
    }else{
      draw_point(buf,p1,c);
      return;
    }
  }
  ratio = (p1.y-p2.y)/((float)(p1.x-p2.x));
  if(DIST(p1.x,p2.x)>DIST(p1.y,p2.y)){
    min = p1.x>p2.x?p2.x:p1.x;
    max = p1.x>p2.x?p1.x:p2.x;   
    for(x=min;x<=max;x++){
      y = (int)(ratio*(x-p1.x)+p1.y);
      p.x = x;
      p.y = y;
      draw_point(buf,p,c);
    }      
  }else{
    ratio = 1.0/ratio;
    min = p1.y>p2.y?p2.y:p1.y;
    max = p1.y>p2.y?p1.y:p2.y;   
    for(y=min;y<=max;y++){
      x = (int)(ratio*(y-p1.y)+p1.x);
      p.x = x;
      p.y = y;
      draw_point(buf,p,c);
    }      
  }
}
void draw_poly(char *buf,point p1,point p2,color c){
  point p3, p4;
  p3.y = p1.y; p3.x = p2.x;
  p4.y = p2.y; p4.x = p1.x;
  draw_line(buf,p1, p3, c);
  draw_line(buf,p3, p2, c);
  draw_line(buf,p2, p4, c);
  draw_line(buf,p4, p1, c);
}
void draw_mark(char *buf,point porg, color c, int val){ //draw figures in 10*16 pixels
  int i,j;
  char vline;
  point p,q;
  p.x = porg.x-5;
  p.y = porg.y-8;
  for(i=0;i<5;i++){
    vline = pixelmap[val*5+i];
    for(j=0;j<8;j++){
      if(vline&(0x80>>j)){
        draw_point(buf,p, c);
        q.x = p.x+1;q.y=p.y;
        draw_point(buf,q, c);
        q.x = p.x;q.y=p.y+1;
        draw_point(buf,q, c);
        q.x = p.x+1;q.y=p.y+1;
        draw_point(buf,q, c);
      }
      p.y+=2;
    }
    p.y = p.y - 16;
    p.x+=2;
  }
}
void draw_rect(char *buf,int x, int y, int width, int height, color c){
  int i,j,pos;
  for(i=x;i<x+width;i++)
    for(j=y;j<y+height;j++){
      pos = (j*fw+i)*fc;
      buf[pos+0] = c.B;
      buf[pos+1] = c.G;
      buf[pos+2] = c.R;
    }
}
/* type0-station type1-entry point type2-route node type3-parking lot */
void draw_vertex_bg(int x, int y, int type){
  switch(type){
    case 0:
      draw_rect(fbg,x-4,y-4,9,9,C_W);
      draw_rect(fbg,x-3,y-3,7,7,C_K);
      break;
    case 1:
      draw_rect(fbg,x-1,y-1,3,3,C_K);
      break;
    case 2:
      draw_rect(fbg,x-1,y-1,3,3,C_K);
      break;
    case 3:
      draw_rect(fbg,x-5,y-5,11,11,C_G);
      break;
    default:
      break;
  }
}
void draw_edge_bg(int x1,int y1, int x2, int y2){
//  color c = rand_color();
  point p1,p2;
  p1.x = x1;
  p1.y = y1;
  p2.x = x2;
  p2.y = y2;
  draw_line(fbg,p1,p2,C_K);
}
void draw_edge(int x1,int y1, int x2, int y2){
  point p1,p2;
  p1.x = x1;
  p1.y = y1;
  p2.x = x2;
  p2.y = y2;
  draw_line(fbuf,p1,p2,C_R);
}
/* payload0:silicon-GREEN 
 * payload1:Empty Dry-Yellow
 * payload2:Empty Wet-Blue
 * payload3:Empty Agv-Black
 * payload4:Empty FLJ-Red */
void draw_AGV(int x,int y,int payload){
  color c;
  if(payload==0) c = C_G;
  else if(payload==1) c = C_Y;
  else if(payload==2) c = C_B;
  else if(payload==3) c = C_K;
  else if(payload==4) c = C_R;
  else c = C_W;
  draw_rect(fbuf,x-2,y-2,5,5,c);
}
/* upleft-unload state
 * downright-load state
 * state:
 *    0 - starving, Red
 *    1 - executing,Yellow
 *    2 - normal ,  Green
 *   -1 - don't have this action, Black
 * AGV:
 *    0 - no agv
 *    1 - booked
 *    2 - docking
 *    3 - pending
 *    4 - leaving
 *   */
void draw_station(int x, int y, int unload, int load,int agv){
  int i,j;
  point p;
  color c;
  point p1,p2;
  if(unload!=-1){
    if(unload==0) c = C_R;
    else if(unload==1) c = C_Y;
    else if(unload==2) c = C_G;
    else exit(0);
    for(i=0;i<6;i++)
      for(j=0;j<(6-i);j++){
        p.x = x-3+j;
        p.y = y-3+i;
        draw_point(fbuf,p,c);
      }
  }
  if(load!=-1){
    if(load==0) c = C_R;
    else if(load==1) c = C_Y;
    else if(load==2) c = C_G;
    else exit(0);
    for(i=1;i<7;i++)
      for(j=7-i;j<7;j++){
        p.x = x-3+j;
        p.y = y-3+i;
        draw_point(fbuf,p,c);
      }
  }
  if(agv!=0){
    if(agv==1){
      p1.x = x+4; p1.y = y+4;
      p2.x = x+9; p2.y = y+9;
      draw_poly(fbuf,p1,p2,C_K);
    }else{
      if(agv==2) c = C_G;
      else if(agv==3) c = C_Y;
      else if(agv==4) c = C_R;
      draw_rect(fbuf,x+4,y+4,5,5,c);
    }
  }
}
