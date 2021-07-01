#ifndef LevelSensors_h
#define LevelSensors_h

#include <Arduino.h>
#include "PCF8574.h"

class LevelSensors {
  public:
    LevelSensors();
    LevelSensors(PCF8574 *expander, void (*levelCallback)(unsigned int));
    int getTankLevel();
    boolean enoughWater();
    void handleIRQ();
    void loop();
  private:
    PCF8574 *m_expander;
    void (*m_levelCallback)(unsigned int);
    int m_currentLevel=0;
    volatile boolean m_doScan = false;
    boolean m_init = false;
};

#endif
