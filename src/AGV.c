#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include "station.h"
#include "AGV.h"

pAGV AGV_dispatch(){
  pAGV res;
  res = (pAGV)malloc(sizeof(AGV));
  res->x = res->y = res->deltaX = res->deltaY = res->v_idx = 0;
  res->inst = res->station = NULL;
  return res;
}
void AGV_parking(pAGV v){
  free(v);
}
void AGV_enter_route(pAGV v){ //send mission to AGV to start
}
void AGV_execute(pAGV v){ //simulation only, real AGV would use callback leave_route
}
void AGV_leave_route(pAGV v){ //receive ack from AGV to finish
}
