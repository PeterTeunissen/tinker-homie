#include "TempSensor.h"

TempSensor::TempSensor(void (*tempCallback)(float temp), DallasTemperature *sensor, int intervalMillis) {
  m_tempCallback = tempCallback;
  m_sensors = sensor;
  m_interval = intervalMillis;
}

void TempSensor::loop() {

  if ((millis()-m_lastSent) > m_interval || (m_init==false)) {

    m_sensors->requestTemperatures();
    float temp = m_sensors->getTempFByIndex(0);

    if ((temp!=m_lastTemp) || (m_init==false)) {
      m_tempCallback(temp);
    }
    m_lastSent = millis();
  }
  
  m_init = true;
}
