#include "HealthHandler.h"
#include <WiFi.h>

#define HEALTH_INTERVAL 30000

HealthHandler::HealthHandler(){
}

HealthHandler::HealthHandler( void (*healthCallback)(unsigned int upTime, unsigned int rssi), 
                              void (*secondCallback)(), 
                              void (*rebootCallback)(), 
                              NTPClient *ntp) {

  m_healthCallback = healthCallback;
  m_secondCallback = secondCallback;
  m_rebootCallback = rebootCallback;
  m_ntp = ntp;
}

void HealthHandler::serverPing() {
  m_lastPing = millis();
}

void HealthHandler::loop() {

  if (millis()-m_lastPing > (5*60*1000)) {
    // reboot.
    Serial.println("Not getting ping commands from server in time. Rebooting...");
    m_rebootCallback();
  }
  
  if (millis()-m_lastHealth>HEALTH_INTERVAL || m_init==false) {
    m_lastHealth = millis();
    m_healthCallback(millis(), rssiToPercentage(WiFi.RSSI()));
  }

  if (m_ntp->getSeconds()!=m_lastSec || m_init==false) {
    m_lastSec = m_ntp->getSeconds();
    m_secondCallback();
  }

  m_init = true;
}

uint8_t HealthHandler::rssiToPercentage(int32_t rssi) {
  uint8_t quality;
  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }

  return quality;
}
