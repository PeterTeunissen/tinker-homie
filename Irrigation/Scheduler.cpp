#include "Scheduler.h"

Scheduler::Scheduler(AlertHandler *alertHandler) {
  m_alertHandler = alertHandler;
}

void Scheduler::init() {
  m_alertHandler->alert(1, "System started.");  
}

void Scheduler::scheduleZones(int runTimes[4], int gracePeriod, int minimumFlow) {

  for(int i=0;i<4;i++) {
    if (m_runTimes[i]!=0) {
      m_alertHandler->alert(2, "Scheduler is busy!");
      Serial.println("Error 2, Scheduler is busy!");
      return;
    }
  }
  
  for(int i=0;i<4;i++) {
    m_runTimes[i]=runTimes[i];
    m_zones[i]->clearError();
    m_zones[i]->setGracePeriod(gracePeriod);
    m_zones[i]->setMinimumFlow(minimumFlow);
  }

  m_alertHandler->alert(3, "Schedule started.");
  m_alertHandler->setRunning(true);
}


void Scheduler::stopSchedule() {
  for(int i=1;i<=m_zoneCount;i++) {
    m_runTimes[i-1]=0;
    m_zones[i-1]->stopZone();
  }  
  m_prevAllIdle = true;
  m_alertHandler->setRunning(false);
  m_alertHandler->alert(4, "Schedule stopped.");
}

void Scheduler::loop() {
  
  for(int i=1;i<=m_zoneCount;i++) {
    m_zones[i-1]->loop();
  }

  boolean allZonesIdle = allIdle();
  boolean anyZoneHasError = anyZoneError();
  boolean zoneStarted = false;

  if (anyZoneHasError) {
    Serial.println("Reset scheduler run times.");
    for(int i=0;i<4;i++) {
      m_runTimes[i] = 0;
    }
  }

  if (allZonesIdle && !anyZoneHasError) {
    for(int i=0;i<4;i++) {
      if (m_runTimes[i]!=0) {
        Serial.print("Start zone:");
        Serial.println(i);
        m_zones[i]->runZone(m_runTimes[i]);
        m_runTimes[i] = 0;
        zoneStarted = true;
        break;
      }
    }
    if (zoneStarted) {
      m_alertHandler->setRunning(true);
    }

    if (anyZoneHasError) {
      m_alertHandler->setRunning(false);      
    }
    
    if (!zoneStarted && m_prevAllIdle!=allZonesIdle && !anyZoneHasError) {
      Serial.println("Alert 5, Schedule Completed.");
      m_alertHandler->alert(5, "Schedule completed.");      
      m_alertHandler->setRunning(false);      
    }
  }

  if (anyZoneHasError) {
    for(int i=0;i<4;i++) {
      m_zones[i]->clearError();
    }
  }
  
  m_alertHandler->loop();
  
  m_prevAllIdle = allZonesIdle;
  m_init = true;
}

boolean Scheduler::anyZoneError() {
  for(int i=0;i<4;i++) {
    if (m_zones[i]->isEndInError()) {
      return true;  
    }
  }
  return false;  
}

boolean Scheduler::allIdle() {
  for(int i=0;i<4;i++) {
    if (m_zones[i]->isDone()==false) {
      return false;  
    }
  }
  return true;
}

void Scheduler::addZone(Zone *zone) {
  m_zoneCount++;
  m_zones[m_zoneCount-1] = zone;
}
