#ifndef _GRAPH_H_
#define _GRAPH_H_

typedef struct _Vertex_{
  int ID;
  int type;
  int x;
  int y;
  char label[16];
  struct _Vertex_ *N,*S,*W,*E;
}Vertex, *pVertex;

typedef struct _Edge_{
  int ID;
  int V1_ID;
  int V2_ID;
}Edge, *pEdge;

typedef struct _Task_{
  int ID;
  int type;
  int SrcV_ID;
  int DstV_ID;
}Task, *pTask;

typedef struct _Route_{
  int ID;
  int T_ID;
  int VCnt;
  int V_ID[40];
  int ECnt;
  int E_ID[40];
  int distance;
  struct _Route_ *prev, *next;
}Route, *pRoute;

int init_graph();
void graph_show_bg();
void test_draw_AGV_station();
void test_draw_route();

#endif
