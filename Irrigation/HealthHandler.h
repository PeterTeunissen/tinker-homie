#ifndef HealthHandler_h
#define HealthHandler_h

#include <Arduino.h>
#include <NTPClient.h>

class HealthHandler {
  public:
    HealthHandler(void (*healthCallback)(unsigned int upTime, unsigned int rssi), void (*secondCallback)(), NTPClient *ntp);
    void loop();
  private:
    void (*m_healthCallback)(unsigned int upTime, unsigned int rssi);
    void (*m_secondCallback)();
    NTPClient *m_ntp;
    unsigned int m_lastHealth = 0;
    unsigned int m_upTime = 0;
    unsigned int m_lastSec = 0;
    uint8_t rssiToPercentage(int32_t rssi);
    boolean m_init = false;
};

#endif
