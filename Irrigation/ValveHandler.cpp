#include "ValveHandler.h"

ValveHandler::ValveHandler(){  
}

ValveHandler::ValveHandler( int zoneId, 
                            int gpioPin, 
                            void (*valveCallback)(unsigned int zoneId, boolean isOn, int timeLeft)) {

  m_valveCallback = valveCallback;
  m_gpioPin = gpioPin;
  m_zoneId = zoneId;
}

void ValveHandler::setRunTimeLeft(int timeLeft) {
  m_timeLeft = timeLeft;
}

void ValveHandler::loop() {
  if (m_currentState!=m_lastSent || m_init==false || m_timeLeft!=m_lastTimeLeftSent) {
    m_valveCallback(m_zoneId, m_currentState, m_timeLeft);    
    m_lastSent = m_currentState;
    m_lastTimeLeftSent = m_timeLeft;
  }
  m_init = true;
}

void ValveHandler::valveOpen() {
  digitalWrite(m_gpioPin, HIGH);
  m_currentState = true;
}

void ValveHandler::valveClose() {
  digitalWrite(m_gpioPin, LOW);
  m_currentState = false;
}
