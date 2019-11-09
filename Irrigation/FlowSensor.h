#ifndef FlowSensor_h
#define FlowSensor_h

#include <Arduino.h>

class FlowSensor {
  public:
    FlowSensor(void (*flowCallback)(unsigned int flow));
    void handleIRQ();
    void loop();
    unsigned int getFlow();
  private:
    void (*m_flowCallback)(unsigned int);
    volatile unsigned int m_count = 0;
    int m_lastTime = 0;
    unsigned int m_lastFlow = 0;
	  unsigned int m_flow = 0;
    boolean m_init = false;
};

#endif
