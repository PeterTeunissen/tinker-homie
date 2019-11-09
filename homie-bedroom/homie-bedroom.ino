/*
 *   Tested with "WiFi Smart Socket ESP8266 MQTT"
 *   and "Sonoff - WiFi Wireless Smart Switch ESP8266 MQTT"
 *
 *   The Relay could be toggeled with the physical pushbutton
*/

#include <Homie.h>
#include <stdio.h>

const int PIN_PIR = 14; // D5
const int PIN_BUTTON = 2; // D4
const int PIN_RELAY = 0; // D3
const int PIN_LED = LED_BUILTIN;

unsigned long pirOnTime = 0;
bool lastPirState = false;
byte pirHandled = 0;

HomieNode stripNode("strip", "strip");
HomieNode pirNode("pir", "pir");

bool stripOnHandler(HomieRange range, String value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  stripNode.setProperty("activate").send(value);
  Homie.getLogger() << "Strip is " << (on ? "on" : "off") << endl;

  return true;
}

void loopHandler() {
  bool pirState = (digitalRead(PIN_PIR) == HIGH);
  if (pirState != lastPirState ) {
    Homie.getLogger() << "PIR sensor is " << (pirState? "on" : "off") << " pre-send" << endl;
    pirNode.setProperty("on").send(pirState ? "true" : "false");
    Homie.getLogger() << "PIR sensor is " << (pirState? "on" : "off") << " post-send" << endl;
    lastPirState = pirState;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_PIR, INPUT);
  digitalWrite(PIN_RELAY, LOW);

  Homie_setFirmware("bedroom-os", "1.0.0");
  Homie.setLoopFunction(loopHandler);
  Homie.setLedPin(PIN_LED, LOW); // setResetTrigger(PIN_BUTTON, LOW, 5000);

  stripNode.advertise("activate").settable(stripOnHandler);

  Homie.setup();
  Homie.getLogger() << "Setup done" << endl;

}

void loop() {
  Homie.loop();
}
