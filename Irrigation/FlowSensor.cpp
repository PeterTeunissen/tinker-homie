#include "FlowSensor.h"

FlowSensor::FlowSensor(void (*flowCallback)(unsigned int flow)) {
  m_flowCallback = flowCallback;
}

unsigned int FlowSensor::getFlow() {
  return m_flow;
}

void FlowSensor::handleIRQ() {
  m_count++;
}

void FlowSensor::loop() {

  if (millis() - m_lastTime > 1000) {
    m_flow = m_count;
    m_count = 0;
    m_lastTime = millis();    
  }
  
  if (m_init==false || m_flow!=m_lastFlow) {
    m_flowCallback(m_flow);
    m_lastFlow = m_flow;
  }
  m_init = true;
}
