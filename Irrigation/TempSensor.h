#ifndef TempSensor_h
#define TempSensor_h

#include <Arduino.h>
#include <DallasTemperature.h>

class TempSensor {
  public:
    TempSensor();
    TempSensor(void (*tempCallback)(float temp), DallasTemperature *sensor, int interval);
    void loop();
  private:
    void (*m_tempCallback)(float temp);
    DallasTemperature *m_sensors;
    float m_lastTemp = 0.0;
    int m_interval;
    int m_readStart = 0;
    int m_lastSent = 0;
    boolean m_init = false;
    boolean m_pendingRead = false;
};

#endif
