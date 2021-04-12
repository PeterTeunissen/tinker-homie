#include "LoraComm.h"

LoraComm::LoraComm(Stream *port,void (*onMessageCb)(int address, char* msg, int snr, int rssi)) {
}

void LoraComm::initNetwork(int networkId, int address, char *cpin) {
  // send AT+NETWORDID=
  // send AT+ADDRESS=
  // send AT+CPIN=
}

void LoraComm::begin() {
  
}

void LoraComm::setVerbose(boolean verbose){
  
}

void LoraComm::setResend(byte maxRetries, int resendWaitMillis){
  
}

void LoraComm::rawSend(int address, char *msg){
  
}

int LoraComm::send(int toAddress, char *msg, boolean needAck){
    // returns the messageId it assigns
    // sends msg
    // crc = calculateCrc(msg)
    // Message m = new Message(toAddress,msg,++m_msgId,maxRetries);
    // rawSend("AT+SEND=" + m.toAddress() + "," + m.msgAndCrc())
    //if (needAck) {
    //    set waiting=true, and start waitTimer
    //}  
}

void LoraComm::loop() {
    // check m_serial.available()
    // build received msg
    // parse
        // +ERR=1
        // +ERR=2
        // +NETWORKID=nnnnnn
        // +ADDRESS=nnnnnn
        // +VER=
        // +UID=
        // +PARAMETER=a,b,c,d
        // +CPIN=
        // +RCV=3,7,ACK:123,12,-34
        //        <sender>,<length>,<msg>,<rssi>,<snr>
        // if msg = ACK then mark msg as received {
        //   get the msgId from the ACK message
        //   set the Message with (msgId) to ACKed
        //   waiting = false
        // }
        // if not ACK: 
        //   parse appart
        //   check crc
        //   if crc OK:
        //     call cb(fromAddress, msg, rssi, snr)
        //   else 
        //     log error

    // if waiting for ACK and timeout 
    //   if m.tries < maxRetries:
    //      send it again
    //      m.tries++
    //      restart waitTimer
    //   else:
    //     log tries exceeded
    //     waiting = false
  
}

LoraMessage::LoraMessage() {
}

LoraMessage::LoraMessage(int toAddress, char *msg, int msgId, boolean needsAck) {
//    m_toAddress = toAddress;
//    m_msg = msg;
//    m_msgId = msgId;
//    m = msg + "," + msgId;
//    crc = calculateCrc(m);
//    m_msgAndCrc = (m + "," + crc).length()+ "," + m + "," + crc;
    // <msg>,<msgId>,<crc>
}

boolean LoraMessage::addReceived(char *s) {
    // address
    // snr
    // rssi
    // crcOK
    // msgId
    // message      
    return false;
}

void LoraMessage::init() {

}
