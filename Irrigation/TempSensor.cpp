#include "TempSensor.h"

TempSensor::TempSensor(void (*tempCallback)(float temp), DallasTemperature *sensor, int intervalMillis) {
  m_tempCallback = tempCallback;
  m_sensors = sensor;
  m_interval = intervalMillis;
}

void TempSensor::loop() {

  if (m_init==false) {
    // Set to 9 bit precision, which is 1/2 degrees F.
    m_sensors->setResolution(9);  
  }

  // Wait 500 ms after we start the conversion, before we try to read the temp.
  if (m_pendingRead && (millis()-m_readStart > 500)) {

    float temp = m_sensors->getTempFByIndex(0);
    Serial.print("Temp Read:");
    Serial.println(temp);

    // If the sensor reports -127C it means the sensor had an error. So we should ignore the value.
    // -127C equals -196.6F. So checking against -190 should be fine.
    
    if (temp<-190.0) {
      Serial.println("Skipping temp value. Will wait for next cycle.");
      m_pendingRead = false;
      return;  
    }
    
    if (temp!=m_lastTemp) {
      m_tempCallback(temp);
      m_lastTemp = temp;
    }
    m_lastSent = millis();
    m_pendingRead = false;
  }
  
  if (((m_pendingRead==false) && (millis()-m_lastSent) > m_interval) || (m_init==false)) {

    Serial.println("Initiate Temp Read");

    m_sensors->setWaitForConversion(false);
    m_sensors->requestTemperatures();
    m_sensors->setWaitForConversion(true);

    m_pendingRead = true;
    m_readStart = millis();
  }
  
  m_init = true;
}
