#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fb_draw.h"
#include "config.h"
#include "graph.h"

Vertex V[GRAPH_VERTICES_COUNT];
Edge E[GRAPH_EDGES_COUNT];
Task T[TASK_COUNT];
int G[GRAPH_VERTICES_COUNT][GRAPH_VERTICES_COUNT];
Route R[ROUTE_COUNT];

int init_graph(){
  int i,j;
  pVertex v1,v2;
  FILE *fp = NULL;
  /* read in vertices.txt to V[] */
  fp = fopen("config/vertices.txt","r");
  if(fp==NULL) return -1;
  for(i=0;i<GRAPH_VERTICES_COUNT;i++){
    fscanf(fp,"%d",&j);
    V[j].ID = j;
    fscanf(fp,"%d %d %d %s",&(V[j].type),&(V[j].x),&(V[j].y),V[j].label);
  }
  /* read in edges.txt to V[], E[] and G[] */
  for(i=0;i<GRAPH_VERTICES_COUNT;i++)
    for(j=0;j<GRAPH_VERTICES_COUNT;j++) G[i][j] = -1;
  fp = fopen("config/edges.txt","r");
  if(fp==NULL) return -1;
  for(i=0;i<GRAPH_EDGES_COUNT;i++){
    fscanf(fp,"%d",&j);
    E[j].ID = j;
    fscanf(fp,"%d %d",&(E[j].V1_ID),&(E[j].V2_ID));
    if(E[j].V1_ID==-1) continue;
    G[E[j].V1_ID][E[j].V2_ID] = G[E[j].V2_ID][E[j].V1_ID] = j;
    v1 = &(V[E[j].V1_ID]);
    v2 = &(V[E[j].V2_ID]);
    if(v1->x == v2->x){
      if(v1->y > v2->y){
        v1->N = v2;
        v2->S = v1;
      }else if(v1->y < v2->y){
        v1->S = v2;
        v2->N = v1;
      }else exit(0);
    }else if(v1->y == v2->y){
      if(v1->x > v2->x){
        v1->W = v2;
        v2->E = v1;
      }else if(v1->x < v2->x){
        v1->E = v2;
        v2->W = v1;
      }else exit(0);
    }else exit(0);
  }
  /* read in tasks.txt to T[] */
  fp = fopen("config/tasks.txt","r");
  if(fp==NULL) return -1;
  for(i=0;i<TASK_COUNT;i++){
    fscanf(fp,"%d",&j);
    T[j].ID = j;
    fscanf(fp,"%d %d %d",&(T[j].SrcV_ID),&(T[j].DstV_ID),&(T[j].type));
  }
  /* read in routes.txt to R[], delete this after in find_route binary */
  fp = fopen("config/routes.txt","r");
  if(fp==NULL) return -1;
//  int VCountMax = 0;
  for(i=0;i<ROUTE_COUNT;i++){
    R[i].ID = i;
    fscanf(fp,"%d %d %d",&(R[i].T_ID),&(R[i].distance),&(R[i].VCnt));
//    if(R[i].VCnt>VCountMax) VCountMax = R[i].VCnt;
    R[i].ECnt = R[i].VCnt-1;
    for(j=0;j<R[i].VCnt;j++) fscanf(fp,"%d",&(R[i].V_ID[j]));
    for(j=0;j<R[i].ECnt;j++) R[i].E_ID[j]=G[R[i].V_ID[j]][R[i].V_ID[j+1]];
  }
//  printf("VCnt Max:%d\n",VCountMax);
  return 0;
}

void graph_show_bg(){
  int i;
  for(i=0;i<GRAPH_VERTICES_COUNT;i++)
    draw_vertex_bg(V[i].x,V[i].y,V[i].type);
#ifdef SHOW_EDGE
  for(i=0;i<GRAPH_EDGES_COUNT;i++)
    if(E[i].V1_ID!=-1)
      draw_edge_bg(V[E[i].V1_ID].x,V[E[i].V1_ID].y,V[E[i].V2_ID].x,V[E[i].V2_ID].y);
#endif
#ifdef SHOW_TASK
  for(i=0;i<TASK_COUNT;i++)
    draw_edge_bg(V[T[i].SrcV_ID].x,V[T[i].SrcV_ID].y,V[T[i].DstV_ID].x,V[T[i].DstV_ID].y);
#endif
}

void test_draw_AGV_station(){
  //test draw_AGV()
  draw_AGV(10,10,0);
  draw_AGV(20,10,1);
  draw_AGV(30,10,2);
  draw_AGV(40,10,3);
  draw_AGV(50,10,4);
  //test draw_station()
  draw_station(V[0].x,V[0].y,0,0,0);
  draw_station(V[1].x,V[1].y,0,1,1);
  draw_station(V[2].x,V[2].y,0,2,2);
  draw_station(V[3].x,V[3].y,0,-1,3);
  draw_station(V[4].x,V[4].y,1,0,4);
  draw_station(V[5].x,V[5].y,1,1,0);
  draw_station(V[6].x,V[6].y,1,2,1);
  draw_station(V[7].x,V[7].y,1,-1,2);
  draw_station(V[8].x,V[8].y,2,0,3);
  draw_station(V[9].x,V[9].y,2,1,4);
  draw_station(V[10].x,V[10].y,2,2,0);
  draw_station(V[11].x,V[11].y,2,-1,1);
  draw_station(V[12].x,V[12].y,-1,0,2);
  draw_station(V[13].x,V[13].y,-1,1,3);
  draw_station(V[14].x,V[14].y,-1,2,4);
  draw_station(V[15].x,V[15].y,-1,-1,0);
}

void test_draw_route(){
  static int RID=0;
  static int old_tid = -1;
  static int tid_r_cnt = 0;
  int i;
  draw_edge(V[T[R[RID].T_ID].SrcV_ID].x,V[T[R[RID].T_ID].SrcV_ID].y,V[T[R[RID].T_ID].DstV_ID].x,V[T[R[RID].T_ID].DstV_ID].y);
  for(i=0;i<R[RID].ECnt;i++)
    draw_edge(V[E[R[RID].E_ID[i]].V1_ID].x,V[E[R[RID].E_ID[i]].V1_ID].y,V[E[R[RID].E_ID[i]].V2_ID].x,V[E[R[RID].E_ID[i]].V2_ID].y);
/*  if(R[RID].T_ID!=old_tid){
    old_tid = R[RID].T_ID;
    tid_r_cnt = 1;
  }
  if(tid_r_cnt < 5){
    RID++;
    tid_r_cnt++;
  }
  else{
    while(R[++RID].T_ID == old_tid) ;
  }*/
  RID++;
  if(RID==ROUTE_COUNT) RID = 0;
}

