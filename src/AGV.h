#ifndef _AGV_H_
#define _AGV_H_

typedef struct _AGV_{
  int x;
  int y;
  int deltaX;
  int deltaY;
  int v_idx;
}AGV,*pAGV;

pAGV AGV_dispatch();
void AGV_parking(pAGV v);
void AGV_enter_route(pAGV v); //send mission to AGV to start
void AGV_execute(pAGV v); //simulation only, real AGV would use callback leave_route
void AGV_leave_route(pAGV v); //receive ack from AGV to finish


#endif
