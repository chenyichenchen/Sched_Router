#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct _Slice_{
  int st;
  int fn;
  struct _Slice_ *prev,*next;
}Slice,*pSlice;

pSlice new_slice(int st,int fn);
pSlice new_slice_list();
void free_slice_list(pSlice s); // clean the entire list
void empty_slice_list(pSlice s); // clean the list except the head
void slice_print(pSlice s);
pSlice slice_shift(int t,pSlice s); //t time has past, adjust list of slices
pSlice slice_intersect(pSlice s1, pSlice s2); //intersection of two lists of slices
pSlice slice_union(pSlice s1, pSlice s2); //union of two lists of slices
pSlice slice_complement(pSlice s); //complementary set of list of slices

typedef struct _Inst_{
  int time_dispatch; //the GCLK when this Inst enter issueQ
  int time_issue; //the GCLK when this Inst enter executeQ
  int time_deadline; //should finish before time_dispatch+time_deadline
  pTask task;
  pAGV carrier;
  pRoute route;
  struct _Inst_ *prev, *next;
}Inst,*pInst;

void schedule();//find pending AGVs and dispatch them
void dispatch(int tid,pAGV v,int deadline); //assign an AGV a task, enter issueQ, TODO:AGV_ID
void issue(); //cross validate and pick routes for insts in issueQ, start to execute
void execute(); //update moving AGVs and show on framebuffer

int init_scheduler();
void fini_scheduler();

#endif
