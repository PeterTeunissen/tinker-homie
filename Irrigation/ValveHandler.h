#ifndef ValveHandler_h
#define ValveHandler_h

#include <Arduino.h>

class ValveHandler {
  public:
    ValveHandler();
    ValveHandler(int zoneId, int gpioPin, void (*valveCallback)(unsigned int zoneId, boolean isOn, int timeLeft));
    void loop();
    void setRunTimeLeft(int i);
    void valveOpen();
    void valveClose();
  private:
    void (*m_valveCallback)(unsigned int zoneId, boolean isOn, int timeLeft); 
    int m_gpioPin;
    int m_zoneId;
    boolean m_currentState = false;
    boolean m_lastSent = false;
    boolean m_init = false;
    int m_timeLeft = 0;
    int m_lastTimeLeftSent = 0;
};

#endif
