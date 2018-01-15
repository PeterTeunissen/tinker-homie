#include <stdio.h>
#include <Homie.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS_PIN 4

const int NUM_SENSORS = 5;
const int SENSOR_INTERVAL = 60;
unsigned long lastTimeSent = 0;

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

DeviceAddress probes[] = { 
    { 0x28, 0xFF, 0xCA, 0xC0, 0x90, 0x17, 0x05, 0x7B },
    { 0x28, 0xFF, 0xC2, 0x82, 0x83, 0x17, 0x04, 0x52 },
    { 0x28, 0xFF, 0x94, 0x80, 0x83, 0x17, 0x04, 0x5C },
    { 0x28, 0xFF, 0xE1, 0xD6, 0x80, 0x14, 0x02, 0x5E },
    { 0x28, 0xFF, 0x76, 0x8B, 0x83, 0x17, 0x04, 0x91 }
};

HomieNode analogNode("temperature", "temperature");

void setupHandler() {
  analogNode.setProperty("unit").send("degrees");
  
  // set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  for (int i=0; i<NUM_SENSORS; i++) {
    sensors.setResolution(probes[i], 12);
  }

  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();  
  sensors.setWaitForConversion(true);

}

void loopHandler() {
  if (millis() - lastTimeSent >= SENSOR_INTERVAL * 1000UL || lastTimeSent == 0) {
    lastTimeSent = millis();

    Serial.println("Awake");   

    //Serial.println(sensors.getDeviceCount());   

    for (int i=0; i<NUM_SENSORS; i++) {
      printTemperature(probes[i],i);
      Serial.println();
    }

    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();  
    sensors.setWaitForConversion(true);
    Serial.println("Sleeping...");   
  }
}

void printTemperature(DeviceAddress deviceAddress, int i) {

  float tempC = sensors.getTempC(deviceAddress); 

  Serial.print("Sensor: ");
  Serial.print(i);
  Serial.print(" ");

  if (tempC == -127.00) {
    Serial.print("Error getting temperature  ");
  } else {
    Serial.print("C: ");
    Serial.print(tempC);
    Serial.print(" F: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    char b[30];
    char outstr[15];
    dtostrf(tempC,7,3,outstr);
    sprintf(b,"{\"sensor\": %d, \"temp\": %s }",i,outstr);
    analogNode.setProperty("temperature").send(b);
 }
}

void setup() {
  Serial.begin(115200);
  //Serial << endl << endl;

  Homie_setFirmware("huzzah-temperature", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  analogNode.advertise("unit");
  analogNode.advertise("degrees");

  Homie.setup();
  //setupHandler();
}

void loop() {
  Homie.loop();
  //loopHandler();  
}

