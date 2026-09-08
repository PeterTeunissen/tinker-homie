#ifndef RYLR_LINK_H
#define RYLR_LINK_H

#define LORA_DEBUG

#include <Arduino.h>
#include <SoftwareSerial.h>

enum LinkState {
  STATE_SEARCHING,
  STATE_LOCKED,
  STATE_FAILED
};

// Custom callback definitions
typedef void (*DataCallback)(int senderID, String message, int rssi, int snr);
// LED write callback: passes the requested state for LED1 and LED2
typedef void (*LedWriteCallback)(bool led1State, bool led2State);

class RylrLink {
  public:
    RylrLink(SoftwareSerial& serialPort);
    
    void begin(DataCallback dataCb, LedWriteCallback ledCb, int maxRetries = 5, unsigned long retryInterval = 3000);
    void update();
    void sendMessage(int address, String message);
    void sendMessage(String message);
    int lastSnr();
    int lastRssi();
    int rssiToPercentage(int rawRssi);
    
  private:
    void slurp(String s);
    SoftwareSerial& _serial;
    DataCallback _onDataReceived;
    LedWriteCallback _onLedWrite;
    
    LinkState _currentState;
    int _futureDestination;
    int _retryCount;
    int _maxRetries;
    
    unsigned long _previousPingMillis;
    unsigned long _pingRetryInterval;
    
    // Internal blinking variables handled completely by the library
    unsigned long _previousBlinkMillis;
    long _blinkInterval;
    bool _toggleState;
    
    void parseIncoming(String response);
    void updateLeds();
    int _lastSnr;
    int _lastRssi;
    bool _dumpMe = true;
};

#endif
