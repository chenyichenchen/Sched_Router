#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"
#include "AGV.h"
#include "fb_draw.h"
#include "config.h"
#include "station.h"
#include "scheduler.h"

extern int GCLK;
extern Route R[ROUTE_COUNT];
extern Vertex V[GRAPH_VERTICES_COUNT];
extern Edge E[GRAPH_EDGES_COUNT];
extern Task T[TASK_COUNT];
pSlice C[ROUTE_COUNT][ROUTE_COUNT];
pStation S[GRAPH_VERTICES_COUNT];
int route_index[2][TASK_COUNT]; //TASK ID->ROUTE ID RANGE [  )
pInst issueQ;
int issueQ_size;
pInst executeQ;
int executeQ_size;

pSlice new_slice(int st,int fn){
  pSlice res = (pSlice)malloc(sizeof(Slice));
  res->st = st;
  res->fn = fn;
  res->prev = NULL;
  res->next = NULL;
  return res;
}
pSlice new_slice_list(){
  return new_slice(0,0); // list head
}
void free_slice_list(pSlice s){
  pSlice temp;
  while(s!=NULL){
    temp = s;
    s = s->next;
    free(temp);
  }
}
void empty_slice_list(pSlice s){
  free_slice_list(s->next);
  s->next = NULL;
}
void slice_print(pSlice s){
  int i=0;
  s = s->next;
  while(s!=NULL){
    printf("Slice%d(%d,%d) ",i,s->st,s->fn);
    i++;
    s = s->next;
  }
  printf("\n");
}
pSlice slice_shift(int t,pSlice s){
  pSlice temp,res,curr,iter;
  res = new_slice_list();
  iter = s->next;
  curr = res;
  while(iter!=NULL){
    curr->next = new_slice(iter->st,iter->fn);
    curr->next->prev = curr;
    curr = curr->next;
    iter = iter->next;
  }
  iter=res->next;
  while(iter!=NULL){
    if(iter->fn == -1){ //final slice is open
      if(iter->st>t) iter->st -= t;
      else iter->st = 0;
      iter = iter->next;
    }else{
      if(iter->fn>t){
        iter->fn -= t;
        if(iter->st>t) iter->st -= t;
        else iter->st = 0;
        iter = iter->next;
      }
      else{
        iter->prev->next = iter->next;
        if(iter->next!=NULL) iter->next->prev = iter->prev;
        temp = iter;
        iter = iter->next;
        free(temp);
      }
    }
  }
  return res;
}
pSlice slice_intersect_inner(pSlice s1, pSlice s2){ //intersect only two slices
  int st,fn;
  st = s1->st>s2->st?s1->st:s2->st;
  if(s1->fn==-1&&s2->fn==-1) fn = -1;
  else if(s1->fn==-1) fn = s2->fn;
  else if(s2->fn==-1) fn = s1->fn;
  else fn = s1->fn>s2->fn?s2->fn:s1->fn;
  if(fn==-1||st<fn) return new_slice(st,fn);
  else return NULL;
}
pSlice slice_intersect(pSlice s1, pSlice s2){ //intersect of two slice lists
  pSlice iter1=s1->next,iter2=s2->next;
  pSlice res = new_slice_list();
  pSlice curr = res;
  while(iter1!=NULL&&iter2!=NULL){
    curr->next = slice_intersect_inner(iter1,iter2);
    if(curr->next!=NULL){
      curr->next->prev = curr;
      curr = curr->next;
      if(curr->fn==iter1->fn) iter1 = iter1->next;
      else iter2 = iter2->next;
    }else{
      if(iter1->st>iter2->st) iter2 = iter2->next;
      else iter1 = iter1->next;
    }
  }
  return res;
}
pSlice slice_union(pSlice s1, pSlice s2){
  return NULL;
}
pSlice slice_complement(pSlice s){
  return NULL;
}

int init_scheduler(){
  int i,j,k;
  pSlice s;
  int st,fn;
  /* init route_index */
  route_index[0][0] = 0;
  route_index[1][TASK_COUNT-1]=ROUTE_COUNT;
  for(i=0;i<ROUTE_COUNT-1;i++){
    if(R[i].T_ID!=R[i+1].T_ID){
      route_index[1][R[i].T_ID] = i+1;
      route_index[0][R[i+1].T_ID] = i+1;
    }
  }
  /* read in cross_valid matrix*/
  FILE *fp = fopen("config/cross_valid.txt","r");
  if(fp==NULL) return -1;
  for(i=0;i<ROUTE_COUNT;i++)
    for(j=0;j<ROUTE_COUNT;j++){
      C[i][j] = new_slice_list();
      s = C[i][j];
      fscanf(fp,"%d",&k);
      while(k--){
        fscanf(fp,"%d %d",&st,&fn);
        s->next = new_slice(st,fn);
        s->next->prev = s;
        s = s->next;
      }
    }
  issueQ = (pInst)malloc(sizeof(Inst));
  issueQ_size = 0;
  executeQ = (pInst)malloc(sizeof(Inst));
  executeQ_size = 0;
  fclose(fp);
  return 0;
}

void fini_scheduler(){
  int i,j;
  for(i=0;i<ROUTE_COUNT;i++)
    for(j=0;j<ROUTE_COUNT;j++){
      free_slice_list(C[i][j]);
    }
  free(issueQ);
  free(executeQ);
}

void dispatch(int tid,pAGV v,int deadline){
  pInst iter;
  pInst inst = (pInst)malloc(sizeof(Inst));
  inst->time_dispatch = GCLK;
  inst->time_issue = -1;
  inst->time_deadline = deadline;
  inst->task = T+tid;
  inst->carrier = v;
  inst->carrier->x = V[inst->task->SrcV_ID].x;
  inst->carrier->y = V[inst->task->SrcV_ID].y;
  if(V[inst->task->SrcV_ID].N!=NULL){
    inst->carrier->deltaX = 0;
    inst->carrier->deltaY = -1;
  }else if(V[inst->task->SrcV_ID].S!=NULL){
    inst->carrier->deltaX = 0;
    inst->carrier->deltaY = 1;
  }else if(V[inst->task->SrcV_ID].W!=NULL){
    inst->carrier->deltaX = -1;
    inst->carrier->deltaY = 0;
  }else{
    inst->carrier->deltaX = 1;
    inst->carrier->deltaY = 0;
  }
  inst->carrier->v_idx = 1;
  inst->route = NULL;
  inst->prev = inst->next = NULL;
  iter = issueQ;
  while(iter->next!=NULL) iter = iter->next;
  iter->next = inst;
  inst->prev = iter;
  issueQ_size++;
  station_dispatch(T[tid].SrcV_ID);
}

void issue(){
  int i,j;
  int starving,issued;
  pSlice temp1,temp2,temp3;
  pInst iter1,iter2;
  iter1 = issueQ->next;
  while(iter1!=NULL){
    starving = 1;
    issued = 0;
    for(i=route_index[0][iter1->task->ID];i<route_index[1][iter1->task->ID];i++){
      iter2 = executeQ->next;
      temp1 = new_slice_list();
      temp1->next = new_slice(0,-1);
      temp1->next->prev = temp1;
      while(iter2!=NULL){
        temp2 = slice_shift(GCLK-iter2->time_issue,C[iter2->route->ID][i]);
        temp3 = slice_intersect(temp1,temp2);
        free_slice_list(temp1);
        free_slice_list(temp2);
        temp1 = temp3;
        iter2 = iter2->next;
      }
      if(temp1->next->st == 0){ //issue!
        starving = 0;
        issued = 1;
        iter1->time_issue = GCLK;
        iter1->route = R+i;
        iter2 = iter1->next;
        iter1->prev->next = iter1->next;
        if(iter1->next!=NULL) iter1->next->prev = iter1->prev;
        iter1->next = executeQ->next;
        iter1->prev = executeQ;
        if(executeQ->next!=NULL) executeQ->next->prev = iter1;
        executeQ->next = iter1;
        station_issue(iter1->task->SrcV_ID);
        iter1 = iter2;
        free_slice_list(temp1);
        issueQ_size--;
        executeQ_size++;
        break;
      }else{
        if(GCLK+temp1->next->st+R[i].distance<iter1->time_dispatch+iter1->time_deadline)
          starving = 0;
      }
      free_slice_list(temp1);
    }
//    if(starving==1) break;
    if(issued==0) iter1 = iter1->next;
  }
}

void execute(){
  int i,j;
  int executed;
  pInst temp;
  pInst iter = executeQ->next;
  while(iter!=NULL){
    j = iter->route->ID;
    for(i=0;i<R[j].ECnt;i++)
      draw_edge(V[E[R[j].E_ID[i]].V1_ID].x,V[E[R[j].E_ID[i]].V1_ID].y,V[E[R[j].E_ID[i]].V2_ID].x,V[E[R[j].E_ID[i]].V2_ID].y);
    iter = iter->next;
  }
  iter = executeQ->next;
  while(iter!=NULL){
    executed = 0;
    iter->carrier->x+=iter->carrier->deltaX;
    iter->carrier->y+=iter->carrier->deltaY;
    draw_AGV(iter->carrier->x,iter->carrier->y,iter->task->type);
    if(iter->carrier->x==V[iter->route->V_ID[iter->carrier->v_idx]].x &&
       iter->carrier->y==V[iter->route->V_ID[iter->carrier->v_idx]].y){
      if(iter->carrier->v_idx == iter->route->VCnt-1){ //done!
        temp = iter->next;
        iter->prev->next = iter->next;
        if(iter->next!=NULL) iter->next->prev = iter->prev;
        station_docking(iter->carrier,iter->task->DstV_ID);
        free(iter);
        iter = temp;
        executed = 1;
        executeQ_size--;
      }else{
        if(V[iter->route->V_ID[iter->carrier->v_idx+1]].x>
           V[iter->route->V_ID[iter->carrier->v_idx]].x){
          iter->carrier->deltaX = 1;
          iter->carrier->deltaY = 0;
        }else if(V[iter->route->V_ID[iter->carrier->v_idx+1]].x<
           V[iter->route->V_ID[iter->carrier->v_idx]].x){
          iter->carrier->deltaX = -1;
          iter->carrier->deltaY = 0;
        }else if(V[iter->route->V_ID[iter->carrier->v_idx+1]].y>
           V[iter->route->V_ID[iter->carrier->v_idx]].y){
          iter->carrier->deltaX = 0;
          iter->carrier->deltaY = 1;
        }else{
          iter->carrier->deltaX = 0;
          iter->carrier->deltaY = -1;
        }
        iter->carrier->v_idx++;
      }
    }
    if(executed==0) iter = iter->next;
  }
}

void schedule(){
  int i,j;
  int deadline,dstV,tid;
  int temp1,temp2;
  pAGV v;
  for(i=0;i<GRAPH_VERTICES_COUNT;i++){
    if(S[i]!=NULL){
      if(S[i]->parking==0&&S[i]->pending==NULL) continue;
      deadline = 4000000;
      dstV = -1;
      for(j=0;j<S[i]->task_count;j++){
        temp1 = T[S[i]->task_id[j]].DstV_ID;
        if(station_occupied(temp1)==0){
          temp2 = station_lasttick(temp1)+S[temp1]->latency-GCLK;
          /*
          if(temp2<=0){
            deadline = 0; //already starving
            dstV = temp1;
            tid = S[i]->task_id[j];
            break;
          }else if(temp2<deadline){
            deadline = temp2;
            dstV = temp1;
            tid = S[i]->task_id[j];
          }*/
          if(temp2<deadline){
            deadline = temp2;
            dstV = temp1;
            tid = S[i]->task_id[j];
          }
        }
      }
      if(dstV==-1) continue; 
      v = station_getAGV(i);
      station_booking(v,dstV);
      if(deadline<0) deadline = 0;
      dispatch(tid,v,deadline);
    }
  }
}
