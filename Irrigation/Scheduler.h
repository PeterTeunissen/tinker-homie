#ifndef Scheduler_h
#define Scheduler_h

#include <Arduino.h>
#include "Zone.h"
#include "AlertHandler.h"

class Scheduler {
  public:
    Scheduler(AlertHandler *alertHandler);
    void init();
    void stopSchedule();
    void scheduleZones(int runTimes[4], int gracePeriod, int minimumFlow);
    void addZone(Zone *zone);
    void loop();
  private:
    AlertHandler *m_alertHandler;
    boolean allIdle();
    boolean anyZoneError();
    int m_zoneCount = 0;
    Zone *m_zones[4];
    unsigned int m_runTimes[4] = {0, 0, 0, 0};
    boolean m_init = false;
    boolean m_prevAllIdle = true;
};

#endif
