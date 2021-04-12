#ifndef _LORACOMM_H
#define _LORACOMM_H

#include <Arduino.h>

class LoraComm {
  public:
    LoraComm(Stream *port, void (*onMessageCb)(int address, char* msg, int snr, int rssi));
    void initNetwork(int networkId, int myAddress, char *cpin = "");
    void rawSend(int fromAddress, char *msg);
    int send(int toAddress, char *msg, boolean needAck=true);
    void setVerbose(boolean verbose);
    void setResend(byte maxRetries, int resendWaitMillis);
    void loop();
    void begin();

  private:
    Stream *m_port;
    void (*m_onMessageCb)(int address, char* msg, int snr, int rssi);
    int m_msgId = 0;
    boolean m_verbose;
    byte m_maxRetries;
    int m_resendWaitMillis;
};

class LoraMessage {
  public:
    LoraMessage();
    LoraMessage(int toAddress, char *msg, int msgId, boolean needsAck);
    boolean addReceived(char *s);
    void init();
    
  private:
    int m_snr;
    int m_rssi;
    int m_address;
    char m_msg[100]; 
    boolean m_needsAck = true;
    int m_msgId;
};
#endif
