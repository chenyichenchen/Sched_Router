#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"
#include "config.h"
#include "AGV.h"
#include "scheduler.h"
#include "fb_draw.h"

#define MARGIN 30

int GCLK;
extern Vertex V[GRAPH_VERTICES_COUNT];
extern Route R[ROUTE_COUNT];
int visit_time[2][40];

/* route1 occupies resource during(st1,fn1)
 * route2 occupies resource during(st2,fn2)
 * delay route2,so they don't conflict on this resource
 * */
pSlice shareResource(int st1, int fn1,int st2,int fn2){
  pSlice s;
  pSlice res = new_slice_list();
  int st,fn;
  st = fn1+MARGIN-st2;
  if(st<0) st = 0;
  res->next = new_slice(st,-1);
  res->next->prev = res;
  if(fn2+MARGIN<st1){
    fn = st1-(fn2+MARGIN);
    s = new_slice(0,fn);
    s->prev = res;
    s->next = res->next;
    s->prev->next = s;
    s->next->prev = s;
  }
  return res;
}

pSlice route_cross_valid(int r1_ID,int r2_ID){
  int i,j;
  pSlice temp1;
  pSlice temp2;
  pSlice res = new_slice_list();
  res->next = new_slice(0,-1);
  res->next->prev = res;
  pRoute r1 = R+r1_ID;
  pRoute r2 = R+r2_ID;
  for(i=0,j=0;i<r1->VCnt;i++){
    visit_time[0][i] = j;
    if(i==r1->VCnt-1) break;
    j+=DIST(V[r1->V_ID[i]].x,V[r1->V_ID[i+1]].x)+DIST(V[r1->V_ID[i]].y,V[r1->V_ID[i+1]].y);
  }
  for(i=0,j=0;i<r2->VCnt;i++){
    visit_time[1][i] = j;
    if(i==r2->VCnt-1) break;
    j+=DIST(V[r2->V_ID[i]].x,V[r2->V_ID[i+1]].x)+DIST(V[r2->V_ID[i]].y,V[r2->V_ID[i+1]].y);
  }
  for(i=0;i<r1->VCnt;i++)
    for(j=0;j<r2->VCnt;j++){
      if(r1->V_ID[i]==r2->V_ID[j]){
        temp1 = shareResource(visit_time[0][i],visit_time[0][i],visit_time[1][j],visit_time[1][j]);
        temp2 = slice_intersect(res,temp1);
        free_slice_list(temp1);
        free_slice_list(res);
        res = temp2;
        break;
      }
    }
  for(i=0;i<r1->ECnt;i++)
    for(j=0;j<r2->ECnt;j++){
      if(r1->E_ID[i]==r2->E_ID[j]){
        temp1 = shareResource(visit_time[0][i],visit_time[0][i+1],visit_time[1][j],visit_time[1][j+1]);
        temp2 = slice_intersect(res,temp1);
        free_slice_list(temp1);
        free_slice_list(res);
        res = temp2;
        break;
      }
    }
  return res;
}

int main(){
  int i,j,k;
  pSlice s;
  pSlice iter;
  init_graph();
  FILE *fp = fopen("config/cross_valid.txt","w");
  if(fp==NULL) return -1;
  for(i=0;i<ROUTE_COUNT;i++)
    for(j=0;j<ROUTE_COUNT;j++){
//      printf("(%4d,%4d):",i,j);
      s = route_cross_valid(i,j);
      iter = s;
      k=0;
      while(iter->next!=NULL){
        k++;
        iter = iter->next;
      }
      fprintf(fp,"%d",k);
      iter = s->next;
      while(iter!=NULL){
        fprintf(fp," %d %d",iter->st,iter->fn);
        iter = iter->next;
      }
      fprintf(fp,"\n");
//      slice_print(s);
      free_slice_list(s);
    }
  return 0;
}
