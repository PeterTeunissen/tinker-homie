#include <stdio.h>
#include <Homie.h>

const int LED_PIN = 13;
const int SEND_PIN = 5;
const int BUTTON_PIN = 0;

const int SENSOR_INTERVAL = 30;
unsigned long lastTimeSent = 0;

HomieNode analogNode("temperature", "temperature");

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);
  pinMode(SEND_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(LED_PIN,HIGH);
  Homie_setFirmware("homie-temperature", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setLedPin(LED_PIN,LOW).setResetTrigger(BUTTON_PIN, LOW, 5000);
  analogNode.advertise("temperature");
  
  Homie.setup();
}

void setupHandler() {

  pinMode(A0,INPUT);
  pinMode(SEND_PIN,OUTPUT);
}

void loopHandler() {

  if (millis() - lastTimeSent >= SENSOR_INTERVAL * 1000UL || lastTimeSent == 0) {

    digitalWrite(SEND_PIN,HIGH);
    
    Homie.getLogger() << "Awake" << endl;
    printTemperature();
    
    Homie.getLogger() << "Sleeping..." << endl;

    lastTimeSent = millis();

    digitalWrite(SEND_PIN,LOW);
  }
}

void printTemperature() {

  int atmpC = analogRead(A0);
  double atempC = atmpC / 1024.0 * 3.3;
  atempC = atempC - 0.5;
  atempC = atempC * 100.0;

  double atempF = (atempC * 9.0/5.0) + 32.0;
  
  Homie.getLogger() << " Analog Temp " << atempC << "C " << atempF << "F" << endl;
    
  char b[200];
  byte error = 0;
  char anastrC[15];
  char anastrF[15];

  dtostrf(atempC,7,1,anastrC);
  dtostrf(atempF,7,1,anastrF);
  
  sprintf(b,"{ \"analog_C\": %s, \"analog_F\": %s }", anastrC, anastrF);
  Homie.getLogger() << "Sending: " << b << endl;
  analogNode.setProperty("temperature").send(b);
}

void loop() {
  Homie.loop();
}
