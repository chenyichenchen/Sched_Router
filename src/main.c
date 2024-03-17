#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fb_draw.h"
#include "config.h"
#include "graph.h"
#include "AGV.h"
#include "scheduler.h"
#include "station.h"

int GCLK = 0;
int Storage = 0;

void render(){
  fb_background();
//  test_draw_AGV_station();
//  test_draw_route();
  fb_show();
}

int main(){
  fb_init();
  init_graph();
  init_scheduler();
  init_station();
  graph_show_bg();
//  if(init()!=0)   //read in configure
//    return -1;
  while(1){
    GCLK++;
    fb_background();
    execute();
    issue();
//    dispatch();
    schedule();
    station_update();
    fb_show();
//    render();
    usleep(SIM_SPEED_US);
    if(GCLK>=SIM_GCLK_MAX||Storage>=SIM_STORAGE_MAX) break;
  }
//  report();
//  fini();
  fini_scheduler();
  fb_fini();
  return 0;
}
