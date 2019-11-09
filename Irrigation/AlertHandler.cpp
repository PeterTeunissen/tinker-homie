#include "AlertHandler.h"

AlertHandler::AlertHandler(void (*alertCallback)(boolean isRunning, int errNo, char *msg)) {
  m_alertCallback = alertCallback;
}

void AlertHandler::alert(int errNo, char *msg) {
	m_alertCallback(m_scheduleRunning, errNo, msg);
}

void AlertHandler::setRunning(boolean isRunning) {
  m_scheduleRunning = isRunning;
}

void AlertHandler::loop() {
  m_alertCallback(m_scheduleRunning, 0, ""); 
}
