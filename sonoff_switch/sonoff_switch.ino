
#include <Homie.h>

//const int PIN_RELAY = 12;
const int PIN_LED = 0;
//const int PIN_BUTTON = 0;

HomieNode switchNode("switch", "switch");

bool switchOnHandler(const HomieRange& range, const String& value) {

  Serial << "Switch handler...";

  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  //digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  digitalWrite(PIN_LED, on ? HIGH : LOW);
  switchNode.setProperty("on").send(value);
  Homie.getLogger() << "Switch is " << (on ? "on" : "off") << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial << endl << endl;
  Serial << "Hello!";
  pinMode(PIN_LED, OUTPUT);
  //pinMode(PIN_RELAY, OUTPUT);
  //digitalWrite(PIN_RELAY, LOW);

  Homie_setFirmware("itead-sonoff", "1.0.0");
 // Homie.setLedPin(PIN_LED, LOW);
  //.setResetTrigger(PIN_BUTTON, LOW, 5000);

  switchNode.advertise("on").settable(switchOnHandler);

  Homie.setup();

  Serial << "Setup done";

}

void loop() {
  Homie.loop();
}
