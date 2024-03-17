#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"
#include "config.h"
#include "fb_draw.h"

#define MAX_ROUTE_PER_TASK 30

extern Vertex V[GRAPH_VERTICES_COUNT];
extern Edge E[GRAPH_EDGES_COUNT];
extern Task T[TASK_COUNT];
extern int G[GRAPH_VERTICES_COUNT][GRAPH_VERTICES_COUNT];
char visit_tag[GRAPH_VERTICES_COUNT];
pRoute head,tail;

void copy_route(pRoute src, pRoute dst){
  int i;
  dst->VCnt = src->VCnt;
  for(i=0;i<dst->VCnt;i++)
    dst->V_ID[i] = src->V_ID[i];
  dst->distance = src->distance;
}

void insert_route(pRoute r){
  pRoute iter = head->next;
  while(iter!=NULL&&r->distance>=iter->distance) iter = iter->next;
  if(iter==NULL){
    tail->next = r;
    r->prev = tail;
    r->next = NULL;
    tail = r;
  }else{
    iter->prev->next = r;
    r->prev = iter->prev;
    iter->prev = r;
    r->next = iter;
  }
}

pRoute remove_tail(){
  pRoute res = tail;
  tail = tail->prev;
  tail->next = NULL;
  res->prev = res->next = NULL;
  return res;
}

void find_route(pTask t, pRoute r, pVertex v){
  int i;
  pRoute new_r;
  if(v->ID == t->SrcV_ID){
    r->VCnt = 1;
    r->V_ID[0] = v->ID;
    r->distance = 0;
    visit_tag[v->ID] = 1;
  }else if(v->ID == t->DstV_ID){
    if(r->distance < tail->distance){
      new_r = remove_tail();
      copy_route(r,new_r);
      insert_route(new_r);
    }
  }
  if(v->y > V[t->DstV_ID].y){
    if(v->N!=NULL&&visit_tag[v->N->ID]==0){
      r->V_ID[r->VCnt] = v->N->ID;
      r->distance += v->y-v->N->y;
      r->VCnt++;
      visit_tag[v->N->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->N);
      r->distance -= v->y-v->N->y;
      r->VCnt--;
      visit_tag[v->N->ID] = 0;
    }
  }else{
    if(v->S!=NULL&&visit_tag[v->S->ID]==0){
      r->V_ID[r->VCnt] = v->S->ID;
      r->distance += v->S->y-v->y;
      r->VCnt++;
      visit_tag[v->S->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->S);
      r->distance -= v->S->y-v->y;
      r->VCnt--;
      visit_tag[v->S->ID] = 0;
    }
  }
  if(v->x > V[t->DstV_ID].x){
    if(v->W!=NULL&&visit_tag[v->W->ID]==0){
      r->V_ID[r->VCnt] = v->W->ID;
      r->distance += v->x-v->W->x;
      r->VCnt++;
      visit_tag[v->W->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->W);
      r->distance -= v->x-v->W->x;
      r->VCnt--;
      visit_tag[v->W->ID] = 0;
    }
  }else{
    if(v->E!=NULL&&visit_tag[v->E->ID]==0){
      r->V_ID[r->VCnt] = v->E->ID;
      r->distance += v->E->x-v->x;
      r->VCnt++;
      visit_tag[v->E->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->E);
      r->distance -= v->E->x-v->x;
      r->VCnt--;
      visit_tag[v->E->ID] = 0;
    }
  }
  if(v->y <= V[t->DstV_ID].y){
    if(v->N!=NULL&&visit_tag[v->N->ID]==0){
      r->V_ID[r->VCnt] = v->N->ID;
      r->distance += v->y-v->N->y;
      r->VCnt++;
      visit_tag[v->N->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->N);
      r->distance -= v->y-v->N->y;
      r->VCnt--;
      visit_tag[v->N->ID] = 0;
    }
  }else{
    if(v->S!=NULL&&visit_tag[v->S->ID]==0){
      r->V_ID[r->VCnt] = v->S->ID;
      r->distance += v->S->y-v->y;
      r->VCnt++;
      visit_tag[v->S->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->S);
      r->distance -= v->S->y-v->y;
      r->VCnt--;
      visit_tag[v->S->ID] = 0;
    }
  }
  if(v->x <= V[t->DstV_ID].x){
    if(v->W!=NULL&&visit_tag[v->W->ID]==0){
      r->V_ID[r->VCnt] = v->W->ID;
      r->distance += v->x-v->W->x;
      r->VCnt++;
      visit_tag[v->W->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->W);
      r->distance -= v->x-v->W->x;
      r->VCnt--;
      visit_tag[v->W->ID] = 0;
    }
  }else{
    if(v->E!=NULL&&visit_tag[v->E->ID]==0){
      r->V_ID[r->VCnt] = v->E->ID;
      r->distance += v->E->x-v->x;
      r->VCnt++;
      visit_tag[v->E->ID] = 1;
      if(r->distance<tail->distance) find_route(t,r,v->E);
      r->distance -= v->E->x-v->x;
      r->VCnt--;
      visit_tag[v->E->ID] = 0;
    }
  }
}

int thresh_distance(int v1, int v2){
  int sum=0;
  int v3,v4;
  if(v1>87){
    if(V[v1].N!=NULL) v3 = V[v1].N->ID;
    else if(V[v1].S!=NULL) v3 = V[v1].S->ID;
    else if(V[v1].W!=NULL) v3 = V[v1].W->ID;
    else if(V[v1].E!=NULL) v3 = V[v1].E->ID;
  }else v3 = v1+88;
  if(v2>87){
    if(V[v2].N!=NULL) v4 = V[v2].N->ID;
    else if(V[v2].S!=NULL) v4 = V[v2].S->ID;
    else if(V[v2].W!=NULL) v4 = V[v2].W->ID;
    else if(V[v2].E!=NULL) v4 = V[v2].E->ID;
  }else v4 = v2+88;
  sum += DIST(V[v1].x,V[v3].x)+DIST(V[v1].y,V[v3].y);
  sum += 10*(DIST(V[v3].x,V[v4].x)+DIST(V[v3].y,V[v4].y));
  sum += DIST(V[v2].x,V[v4].x)+DIST(V[v2].y,V[v4].y);
  printf("(%d,%d),(%d,%d),(%d,%d),(%d,%d),thresh distance:%5d ",V[v1].x,V[v1].y,V[v3].x,V[v3].y,V[v2].x,V[v2].y,V[v4].x,V[v4].y,sum);
  return sum;
}

int main(){
  int i;
  int tid; //task ID
  int threshold;
  Route array[MAX_ROUTE_PER_TASK+1];
  FILE *fp = fopen("config/routes.txt","w");
  if(fp==NULL) return -1;
  init_graph();
  for(tid=0;tid<TASK_COUNT;tid++){
    head = array+0;
    tail = array+MAX_ROUTE_PER_TASK;
    head->prev = NULL;
    tail->next = NULL;
    threshold = thresh_distance(T[tid].SrcV_ID,T[tid].DstV_ID);
    for(i=0;i<MAX_ROUTE_PER_TASK;i++){
      array[i].next = array+i+1;
      array[i+1].prev = array+i;
      array[i].distance = array[i+1].distance = threshold;
      array[i].VCnt = 0;
    }
    pTask t = T+tid;
    for(i=0;i<GRAPH_VERTICES_COUNT;i++) visit_tag[i] = 0;
    find_route(t,head,&(V[t->SrcV_ID]));
    pRoute iter = head->next;
    while(iter!=NULL){
      if(iter->VCnt == 0) break;
      fprintf(fp,"%3d %4d %3d",tid,iter->distance,iter->VCnt);
      for(i=0;i<iter->VCnt;i++)
        fprintf(fp," %d",iter->V_ID[i]);
      fprintf(fp,"\n");
      iter = iter->next;
    }
    printf("%d task routes finished!\n",tid);
  }
  fclose(fp);
  return 0;
}
