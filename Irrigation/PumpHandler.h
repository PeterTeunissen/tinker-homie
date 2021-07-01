#ifndef PumpHandler_h
#define PumpHandler_h

#include <Arduino.h>

class PumpHandler {
  public:
    PumpHandler();
    PumpHandler(int gpioPin, void (*pumpCallback)(boolean isOn));
    void pumpOn();
    void pumpOff();
    void loop();
  private:
    void (*m_pumpCallback)(boolean isOn); 
    int m_gpioPin;
    boolean m_currentState = false;
    boolean m_lastSend = false;
    boolean m_init = false;
};

#endif
