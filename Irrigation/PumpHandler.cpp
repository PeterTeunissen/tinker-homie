#include "PumpHandler.h"

PumpHandler::PumpHandler(int gpioPin, void (*pumpCallback)(boolean isOn)) {
  m_pumpCallback = pumpCallback;
  m_gpioPin = gpioPin;
}

void PumpHandler::loop() {
  if (m_lastSend != m_currentState || m_init==false) {
    m_pumpCallback(m_currentState);
  }
      
  m_lastSend = m_currentState;
  m_init = true;
}

void PumpHandler::pumpOn(){
  digitalWrite(m_gpioPin, HIGH);
  m_currentState = true;
}

void PumpHandler::pumpOff(){
  digitalWrite(m_gpioPin, LOW);
  m_currentState = false;
}
