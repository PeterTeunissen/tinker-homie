#ifndef HealthHandler_h
#define HealthHandler_h

#include <Arduino.h>
#include <NTPClient.h>

class HealthHandler {
  public:
    HealthHandler();
    HealthHandler(void (*healthCallback)(unsigned int upTime, unsigned int rssi), void (*secondCallback)(), void (*rebootCallback)(), NTPClient *ntp);
    void loop();
    void serverPing();
  private:
    void (*m_healthCallback)(unsigned int upTime, unsigned int rssi);
    void (*m_secondCallback)();
    void (*m_rebootCallback)();
    NTPClient *m_ntp;
    unsigned int m_lastHealth = 0;
    unsigned int m_lastSec = 0;
    unsigned int m_lastPing = 0;
    uint8_t rssiToPercentage(int32_t rssi);
    boolean m_init = false;
};

#endif
