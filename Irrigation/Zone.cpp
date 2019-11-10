#include "Zone.h"

Zone::Zone(ValveHandler *v, PumpHandler *p, LevelSensors *l, FlowSensor *f, AlertHandler *a, int gracePeriod, int flowMinimum) {
  m_valveHandler = v;
  m_pumpHandler = p;
  m_alertHandler = a;
  m_levelSensors = l;
  m_flowSensor = f;
  m_gracePeriod = gracePeriod;
  m_flowMinimum = flowMinimum;
}

void Zone::loop() {

  m_valveHandler->loop();
  
  if (m_running) {

    if (m_levelSensors->enoughWater()==false) {
      m_endInError = true;
      stopZone();
      m_alertHandler->alert(6, "Tank level too low!");
      Serial.println("Error 6, Tank level too low!");
    } else {
    
      unsigned int onTime = millis()-m_startTime;

      if (onTime>m_gracePeriod && m_flowSensor->getFlow()<m_flowMinimum) {
        m_endInError = true;
        stopZone();
        m_alertHandler->alert(8, "Not enough water flow!");
        Serial.println("Error 8, Not enough water flow");
      }

      // check the running flag again, since it could have been reset by the minimum flow check
      if (m_running) {
        if (onTime>m_maxTime) {
          stopZone();  
          Serial.println("Zone done");
        } else {
          int  t = m_maxTime-onTime;
          m_valveHandler->setRunTimeLeft(t/1000);
        }
      }
    }
  }
}

void Zone::setMinimumFlow(int minFlow) {
  m_flowMinimum = minFlow;
}

void Zone::setGracePeriod(int gracePeriod){
  m_gracePeriod = gracePeriod;
}

boolean Zone::isEndInError() {
  return m_endInError;
}

void Zone::clearError() {
  m_endInError = false;
}

void Zone::stopZone() {
  m_pumpHandler->pumpOff();
  m_valveHandler->valveClose();
  m_valveHandler->setRunTimeLeft(0);
  m_alertHandler->setRunning(false);
  m_running = false;
  m_maxTime = 0;
}

boolean Zone::isDone() {
  return !m_running;
}

void Zone::runZone(int secs) {
  if (m_running || m_levelSensors->enoughWater()==false) {
    m_alertHandler->alert(7, "Tank level too low!");
    Serial.println("Error 7, Tank level too low!");
    return;
  }
  m_endInError = false;
  m_maxTime = secs*1000;
  m_startTime = millis();
  m_valveHandler->setRunTimeLeft(secs);
  m_valveHandler->valveOpen();
  m_pumpHandler->pumpOn();
  m_running = true;
}
