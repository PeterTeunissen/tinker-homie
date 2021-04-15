#ifndef _LORACOMM_H
#define _LORACOMM_H

#include <Arduino.h>
#include <CRC32.h>

class LoraMessage {
  public:
    LoraMessage();
    LoraMessage(int toAddress, char *msg, int msgId, boolean needsAck);
    boolean addReceived(char s);
    void init();
    char* getOutBuffer();
    char* getMessage();
    int getSNR();
    int getRSSI();
    int getAddress();
    int getMsgId();
    int getAckMsgId();
    boolean isAck();
    boolean isAcked();
    boolean isComplete();
    void parse(char *msg);
    void addCharToString(char *s, char c);
    void parsePayload(char *s);
    void parseAck(char *s);
        
  private:
    int m_snr;
    int m_rssi;
    int m_address;
    char m_message[100]; 
    char m_raw[100];
    char m_msgType[30];
    boolean m_needsAck = true;
    boolean m_isAcked = false;
    boolean m_isAck = false;
    boolean m_isComplete = false;
    int m_msgId;
    int m_ackMsgId;
    boolean startsWith(char *s, char *w);
    void removeChars(char *str, byte s);
    int reverseFind(char *s, char c);
};

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
    void parse(char *s);

  private:
    Stream *m_port;
    void (*m_onMessageCb)(int address, char* msg, int snr, int rssi);
    int m_msgId = 0;
    boolean m_verbose;
    byte m_maxRetries;
    int m_resendWaitMillis;
    boolean m_waitingForAck=false;
    unsigned int m_waitStart;
    LoraMessage *m_inMessage = new LoraMessage();
    LoraMessage *m_outMessage;
    char m_buf[150];
};

#endif
