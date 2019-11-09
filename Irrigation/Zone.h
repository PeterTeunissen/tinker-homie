#ifndef Zone_h
#define Zone_h

#include <Arduino.h>
#include "ValveHandler.h"
#include "PumpHandler.h"
#include "LevelSensors.h"
#include "AlertHandler.h"
#include "FlowSensor.h"

class Zone {
  public:
    Zone(ValveHandler *v, PumpHandler *p, LevelSensors *l, FlowSensor *f, AlertHandler *a, int gracePeriod, int flowMinimum);
    void loop();
    void stopZone();
    void runZone(int secs);
    boolean isDone();
    boolean isEndInError();
    void clearError();
  private:
    ValveHandler *m_valveHandler;
    PumpHandler *m_pumpHandler; 
    LevelSensors *m_levelSensors;
    AlertHandler *m_alertHandler;
    FlowSensor *m_flowSensor;
    int m_flowMinimum;
    int m_gracePeriod;
    int m_trailTime = 0;
    unsigned int m_startTime;
    int m_maxTime;
    int m_running = false;
    boolean m_init = false;
    boolean m_endInError = false;
};

#endif
