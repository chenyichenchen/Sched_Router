#ifndef _STATION_H_
#define _STATION_H_

typedef struct _Station_{
  char label[12];
  int parking; //1 if this station is parking lot
  int load_latency; //heartbeat latency for load
  int load_state; // 0-starving 1-loading 2-counting down -1-not valid
  int load_count;
  int load_valid; //0->1 after loading, 1->0 if both load_valid and unload_valid are 1
  int unload_latency; //heartbeat latency for unload
  int unload_state; // 0-starving 1-unloading 2-counting down -1-not valid
  int unload_count;
  int unload_valid; //0->1 after unloading, 1->0 if both load_valid and unload_valid are 1
  //lasttick+latency is the predicted GCLK for next request of this station
  int lasttick; //last GCLK when this station finish load/unload , (load_valid,unload_valid):(1,1)->(0,0)
  int latency;  //the bigger one in load_latency and unload_latency
  //at most one pointer can be valid for next four data member
  pAGV booking;  // wait for AGV to arrive
  pAGV docking; // wait for station notify server load and unload finished
  pAGV pending; // wait for scheduler to dispatch
  pAGV leaving; // wait for scheduler to issue
  //task start from this station
  int task_count;
  int task_id[10];
}Station, *pStation;

pAGV station_getAGV(int id); //return pending AGV if exists
int station_occupied(int id); //if the station doesn't have AGV occupied or booked
void station_booking(pAGV v, int id);// AGV set station as its target
void station_docking(pAGV v, int id);//AGV arrived at its target station, notify station
void station_pending(int id);//AGV has finished load/unload tasks, station notify server
void station_dispatch(int id);//AGVs enter issueQ
void station_issue(int id);//AGVs enter executeQ
int station_lasttick(int id);//return last production start GCLK
void init_station();
void station_execute(int id);//simulation only, real station would use callback pending
void station_update();//update station load unload states

#endif
