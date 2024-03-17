#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"
#include "AGV.h"
#include "fb_draw.h"
#include "config.h"
#include "scheduler.h"
#include "station.h"

Station _S_[STATION_COUNT];
pStation S[GRAPH_VERTICES_COUNT];
extern int GCLK;
extern Task T[TASK_COUNT];
extern Vertex V[GRAPH_VERTICES_COUNT];

pAGV station_getAGV(int id){ //return pending AGV if exists
  pAGV res;
  if(S[id]->parking==1){  //parking lot always has more AGVs
    res = (pAGV)malloc(sizeof(AGV));
    res->x = res->y = res->deltaX = res->deltaY = res->v_idx = 0;
  }else{
    res = S[id]->pending;
  }
  return res;
}
int station_occupied(int id){ //if the station doesn't have AGV occupied or booked
  if(S[id]->parking==1) return 0; //parking lot always can enter more AGVs
  if(S[id]->booking!=NULL ||
     S[id]->docking!=NULL ||
     S[id]->pending!=NULL ||
     S[id]->leaving!=NULL) return 1;
  return 0;
}
void station_booking(pAGV v, int id){// AGV set station as its target
  if(S[id]->parking==1) return;
  S[id]->booking = v;
}
void station_docking(pAGV v,int id){//AGV arrived at its target station, notify station
  if(S[id]->parking==1){
    free(v);
    return;
  }
  S[id]->docking = S[id]->booking;
  S[id]->booking = NULL;
}
void station_pending(int id){//AGV has finished load/unload tasks, station notify server
  if(S[id]->parking==1) return;
  S[id]->pending = S[id]->docking;
  S[id]->lasttick = GCLK;
  S[id]->docking = NULL;
}
void station_dispatch(int id){//AGVs enter issueQ
  if(S[id]->parking==1) return;
  S[id]->leaving = S[id]->pending;
  S[id]->pending = NULL;
}
void station_issue(int id){//AGVs enter executeQ
  if(S[id]->parking==1) return;
  S[id]->leaving = NULL;
}
int station_lasttick(int id){//return last production start GCLK
  if(S[id]->parking==1) return 0;
  return S[id]->lasttick;
}
void init_station(){
  int i,j;
  pStation sta;
  FILE *fp = fopen("config/station.txt","r");
  if(fp==NULL) exit(-1);
  for(i=0;i<STATION_COUNT;i++){
    fscanf(fp,"%d %d %d %s",&j,&(_S_[i].load_latency),&(_S_[i].unload_latency),_S_[i].label);
    S[j] = _S_+i;
    if(S[j]->load_latency==-1) S[j]->load_state = -1;
    if(S[j]->unload_latency==-1) S[j]->unload_state = -1;
    S[j]->latency = S[j]->load_latency>S[j]->unload_latency?S[j]->load_latency:S[j]->unload_latency;
    if(S[j]->load_state==-1&&S[j]->unload_state==-1) S[j]->parking = 1;
  }
  for(i=0;i<TASK_COUNT;i++){
    sta = S[T[i].SrcV_ID];
    sta->task_id[sta->task_count++] = i;
  }
  fclose(fp);
}

void station_execute(int id){//simulation only, real station would use callback pending
  int i;
  if(S[id]->parking==1) return;
  if(S[id]->load_state == 0){
    if(S[id]->docking!=NULL&&S[id]->load_valid==0){
      S[id]->load_count = DOCKING_LATENCY;
      S[id]->load_state = 1;
    }
  }else if(S[id]->load_state == 1){
    if(S[id]->load_count!=0) S[id]->load_count--;
    else{
      S[id]->load_count = S[id]->load_latency;
      S[id]->load_state = 2;
      S[id]->load_valid = 1;
    }
  }else if(S[id]->load_state == 2){
    if(S[id]->load_count!=0) S[id]->load_count--;
    else{
      S[id]->load_state = 0;
    }
  }else S[id]->load_valid = 1;
  if(S[id]->unload_state == 0&&S[id]->unload_valid==0){
    if(S[id]->docking!=NULL){
      S[id]->unload_count = DOCKING_LATENCY;
      S[id]->unload_state = 1;
    }
  }else if(S[id]->unload_state == 1){
    if(S[id]->unload_count!=0) S[id]->unload_count--;
    else{
      S[id]->unload_count = S[id]->unload_latency;
      S[id]->unload_state = 2;
      S[id]->unload_valid = 1;
    }
  }else if(S[id]->unload_state == 2){
    if(S[id]->unload_count!=0) S[id]->unload_count--;
    else{
      S[id]->unload_state = 0;
    }
  }else S[id]->unload_valid = 1;
  if(S[id]->load_valid==1&&S[id]->unload_valid==1){
    S[id]->load_valid = 0;
    S[id]->unload_valid = 0;
    station_pending(id);
  }
  i = 0;
  if(S[id]->booking!=NULL) i=1;
  else if(S[id]->docking!=NULL) i=2;
  else if(S[id]->pending!=NULL) i=3;
  else if(S[id]->leaving!=NULL) i=4;

  draw_station(V[id].x,V[id].y,S[id]->unload_state,S[id]->load_state,i);
}

void station_update(){
  int i;
  for(i=0;i<GRAPH_VERTICES_COUNT;i++)
    if(S[i]!=NULL) station_execute(i);
}

