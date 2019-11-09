#ifndef AlertHandler_h
#define AlertHandler_h

#include <Arduino.h>

class AlertHandler {
  public:
    AlertHandler(void (*alertCallback)(boolean isRunning, int errNo, char *msg));
    void alert(int errNo, char *msg);
    void loop();
    void setRunning(boolean isRunning);
  private:
    void (*m_alertCallback)(boolean isRunning, int errNo, char *msg); 
    boolean m_scheduleRunning = false;
    boolean m_prevScheduleRunning = false;
};

#endif
