#ifndef _FB_DRAW_H_
#define _FB_DRAW_H_
typedef struct _color_{
  char R;
  char G;
  char B;
}color,*pcolor;

typedef struct _point_{
  int x;
  int y;
}point, *ppoint;

#define DIST(x,y)       ((x)>(y)?((x)-(y)):((y)-(x)))

int fb_init();  /*initialize frame buffer related structures*/
int fb_background(); /*draw background picture in buffer*/
int fb_show();  /*refresh the screen*/
int fb_fini();  /*clean up frame buffer related structures*/
void draw_clear();
void draw_pic(char *pbuf, int width, int height, int channel);
void draw_point(char *buf,point p, color c);
void draw_line(char *buf,point p1, point p2, color c);
void draw_poly(char *buf,point p1, point p2, color c);
void draw_mark(char *buf,point porg, color c, int val);

void draw_vertex_bg(int x,int y,int type);
void draw_edge_bg(int x1,int y1,int x2,int y2);
void draw_edge(int x1,int y1,int x2,int y2);
void draw_AGV(int x,int y,int payload);
void draw_station(int x, int y, int unload, int load, int agv);

#endif
