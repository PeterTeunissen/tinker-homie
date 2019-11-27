#include "LevelSensors.h"

LevelSensors::LevelSensors( PCF8574 *expander, 
                            void (*levelCallback)(unsigned int)) {
  m_expander = expander;
  m_levelCallback = levelCallback;
}

boolean LevelSensors::enoughWater() {
  return getTankLevel()>0;
}

int LevelSensors::getTankLevel() {
  return m_currentLevel;
}

void LevelSensors::handleIRQ() {
  m_doScan = true;
}

void LevelSensors::loop() {
  if (m_doScan || m_init==false) {
    m_doScan = false;
    int sum = 0;
    for(int i=0;i<6;i++) {
      int switchState = m_expander->read(i);         
      if (switchState==0) {
        sum++;
      }
    }
    sum = (sum/6.0 * 100.0);
    
    if (sum!=m_currentLevel || m_init==false) {
      m_currentLevel = sum;
      m_levelCallback(sum);
    }
  }

  m_init = true;
}
