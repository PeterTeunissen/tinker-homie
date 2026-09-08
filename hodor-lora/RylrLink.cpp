#include "RylrLink.h"

RylrLink::RylrLink(SoftwareSerial& serialPort) : _serial(serialPort) {
  _currentState = STATE_SEARCHING;
  _futureDestination = 0;
  _retryCount = 0;
  _maxRetries = 10;
  _previousPingMillis = 0;
  _pingRetryInterval = 3000;
  _dumpMe = true;
  _previousBlinkMillis = 0;
  _blinkInterval = 250; // Initial slow blink interval
  _toggleState = false;
}

int RylrLink::rssiToPercentage(int rawRssi) {
  // 1. Define standard signal thresholds
    const int MIN_DBM = -100; // 0% Quality threshold
    const int MAX_DBM = -30;  // 100% Quality threshold

    // 2. Clamp the raw input using the built-in Arduino/C++ constraint tool
    // This ensures values like -25 dBm don't return over 100%
    #if defined(ARDUINO)
        int clampedRssi = constrain(rawRssi, MIN_DBM, MAX_DBM);
    #else
        int clampedRssi = std::clamp(rawRssi, MIN_DBM, MAX_DBM);
    #endif

    // 3. Execute linear mapping: ((clamped - min) * 100) / (max - min)
    return ((clampedRssi - MIN_DBM) * 100) / (MAX_DBM - MIN_DBM);
}

void RylrLink::begin(DataCallback dataCb, LedWriteCallback ledCb, int maxRetries, unsigned long retryInterval) {
  _onDataReceived = dataCb;
  _onLedWrite = ledCb;
  _maxRetries = maxRetries;
  _pingRetryInterval = retryInterval;
}

void RylrLink::slurp(String cmd) {
  Serial.println();
  Serial.print("[Library] Reply to:");
  Serial.println(cmd);    
  _serial.println(cmd);
  delay(500);
  yield();
  while (_serial.available()>0) {
    String loraData = _serial.readStringUntil('\n');
    Serial.print("[Library] softwareSerial message:");
    Serial.println(loraData);    
  }
}

void RylrLink::update() {

  if (_dumpMe) {
    #ifdef LORA_DEBUG
        Serial.println("RylrLink LORA_DEBUG is ON");
    #else
        Serial.println("RylrLink LORA_DEBUG is OFF");
    #endif
    slurp("AT+ADDRESS?");  
    slurp("AT+NETWORKID?");
    _dumpMe = false;
  }
  
  unsigned long currentMillis = millis();

  // 1. Handle Transmissions & Handshake Retries
  if (_currentState == STATE_SEARCHING) {
    if (_previousPingMillis == 0 || (currentMillis - _previousPingMillis >= _pingRetryInterval)) {
      if (_retryCount < _maxRetries) {                  
        _previousPingMillis = currentMillis;
        _retryCount++;
        #ifdef LORA_DEBUG
          Serial.print("Trying PING number:");
          Serial.println(_retryCount);
        #endif
        _serial.println("AT+SEND=0,4,PING");
      } else {
        _currentState = STATE_FAILED;
        _blinkInterval = 75; // Automatically shift to rapid error blink speed internally
        Serial.println("[Library] Max retries reached. Switching to STATE_FAILED.");
      }
    }
  }

  // 2. Handle Blink Logic Processing Internally
  if (_futureDestination!=0 || _currentState==STATE_SEARCHING || _currentState==STATE_FAILED) {
    updateLeds();
  }

  // 3. Read Incoming RYLR Stream Data
  if (_serial.available()) {
    String response = _serial.readStringUntil('\n');
    response.trim();

    #ifdef LORA_DEBUG
    Serial.print("[Library] incomming lora message:");
    Serial.println(response);    
    #endif
    
    if (response.startsWith("+RCV=")) {
      if (_onLedWrite) {
        _toggleState = !_toggleState;
        _onLedWrite(_toggleState, !_toggleState);
      }
      parseIncoming(response.substring(5));
    }
  }
}

void RylrLink::updateLeds() {
  if (!_onLedWrite) return; // Exit if no callback was registered

  unsigned long currentMillis = millis();

  if (_currentState == STATE_SEARCHING || _currentState == STATE_FAILED) {
    // Process alternating blinking frames using current internal interval rate
    if (currentMillis - _previousBlinkMillis >= _blinkInterval) {
      _previousBlinkMillis = currentMillis;
      _toggleState = !_toggleState;
      // Send the alternating true/false patterns directly to hardware handler
      _onLedWrite(_toggleState, !_toggleState);
    }
  } else if (_currentState == STATE_LOCKED) {
    // Solid on when target locked secured
    // _onLedWrite(true, true);
  }
}

void RylrLink::parseIncoming(String rawData) {

  // +RCV=44,4,PONG,-3,-4
  // Coming into this routine is only "44,4,PONG,-3,-4"
  
  // 2. Find the index of the first comma (ends Address)
  int firstComma = rawData.indexOf(',');
  if (firstComma == -1) return;

  // 3. Find the index of the second comma (ends Length)
  int secondComma = rawData.indexOf(',', firstComma + 1);
  if (secondComma == -1) return;

  // 4. Extract Address and Length
  String senderIDStr = rawData.substring(0, firstComma);
  int senderID = senderIDStr.toInt();
  int dataLength = rawData.substring(firstComma + 1, secondComma).toInt();

  // 5. Use the dataLength to safely isolate the payload
  // The payload starts right after the second comma
  int payloadStart = secondComma + 1;
  String payload = rawData.substring(payloadStart, payloadStart + dataLength);

  // 6. Extract the remaining metadata (RSSI and SNR)
  // These reside after the payload and its trailing comma
  int thirdComma = payloadStart + dataLength; // This points to the comma after the payload
  int fourthComma = rawData.indexOf(',', thirdComma + 1);

  String rssi = "";
  String snr = "";
  
  if (thirdComma < rawData.length()) {
    rssi = rawData.substring(thirdComma + 1, fourthComma);
    if (fourthComma != -1) {
      snr = rawData.substring(fourthComma + 1);
    }
  }
  
  // Extract and convert RSSI and SNR
  _lastRssi = rssiToPercentage(rssi.toInt());
  _lastSnr = snr.toInt();

  if (payload.indexOf("PONG") >= 0 && _currentState == STATE_SEARCHING) {
    _futureDestination = senderID;
    _currentState = STATE_LOCKED;
    Serial.print("[Library] Target Locked! Gateway Address: ");
    Serial.println(_futureDestination);
  } else if (_onDataReceived) {
    #ifdef LORA_DEBUG
      Serial.print("[Library] Calling callback with senderID: ");
      Serial.print(senderID);
      Serial.print(" payload: ");
      Serial.print(payload);      
      Serial.print(" rssi: ");
      Serial.print(_lastRssi);
      Serial.print(" snr: ");
      Serial.println(_lastSnr);
    #endif
    _onDataReceived(senderID, payload, _lastRssi, _lastSnr);
  } else {
    Serial.print("[Library] Got data but not callback! Check code!");
  }
}

void RylrLink::sendMessage(int address, String message){
  _serial.println("AT+SEND=" + String(address) + "," + String(message.length()) + "," + message);
}

void RylrLink::sendMessage(String message){
  sendMessage(_futureDestination, message);
}

int RylrLink::lastSnr() {
  return _lastSnr;
}

int RylrLink::lastRssi() {
  return _lastRssi;
}
