#include <stdio.h>
#include <Homie.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS_PIN 14

const int LED_PIN = 13;
const int BUTTON_PIN = 0;

int NUM_SENSORS = 0;
const int SENSOR_INTERVAL = 60;
unsigned long lastTimeSent = 0;

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

DeviceAddress ds_addr[10];

HomieNode analogNode("temperature", "temperature");

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(LED_PIN,HIGH);
  Homie_setFirmware("homie-temperature", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setLedPin(LED_PIN,LOW).setResetTrigger(BUTTON_PIN, LOW, 5000);
  analogNode.advertise("sensor_count");
  analogNode.advertise("temperature");

  sensors.begin();
  
  Homie.setup();
}

void setupHandler() {

  NUM_SENSORS = sensors.getDeviceCount();

  for(int i=0;i<NUM_SENSORS;i++) {
    sensors.getAddress(ds_addr[i], i);
    sensors.setResolution(ds_addr[i], 12);
  }

  analogNode.advertise("sensor_count");

  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();  
  sensors.setWaitForConversion(true);

}

void loopHandler() {

  if (millis() - lastTimeSent >= SENSOR_INTERVAL * 1000UL || lastTimeSent == 0) {

    Homie.getLogger() << "Awake" << endl;
    digitalWrite(LED_PIN, LOW);

    if (NUM_SENSORS==0) {
      NUM_SENSORS = sensors.getDeviceCount();
      for(int i=0;i<NUM_SENSORS;i++) {
        sensors.getAddress(ds_addr[i], i);
        sensors.setResolution(ds_addr[i], 12);
      }
    }
    
    analogNode.setProperty("sensor_count").send(String(NUM_SENSORS));
    Homie.getLogger() << "Sending: sensor_count:" << String(NUM_SENSORS) << endl;

    for(int i=0;i<NUM_SENSORS;i++) {
      printTemperature(ds_addr[i],i,NUM_SENSORS);
    }
    
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();  
    sensors.setWaitForConversion(true);

    Homie.getLogger() << "Sleeping..." << endl;
    digitalWrite(LED_PIN, HIGH);
    lastTimeSent = millis();
  }
}

void printTemperature(DeviceAddress deviceAddress, int i, int m) {

  float tempC = sensors.getTempC(deviceAddress); 
  Homie.getLogger() << "Sensor: " << i << " ";

  char b[100];
  char s_addr[30];
  char hx[5];
  byte error = 0;
  char outstrC[15];
  char outstrF[15];

  strcpy(s_addr,"");
  for(int i=0;i<8;i++) {
    sprintf(hx,"%02X",deviceAddress[i]);
    if (i!=0) {
      strcat(s_addr,"-");
    }
    strcat(s_addr,hx);
  }
  //sprintf(s_addr,"%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X",deviceAddress[0],deviceAddress[1],deviceAddress[2],deviceAddress[3],deviceAddress[4],deviceAddress[5],deviceAddress[6],deviceAddress[7]);

  if (tempC == -127.00) {
    Homie.getLogger() << "Error getting temperature " << endl;
    //sprintf(b,"{\"sensor\": \"%s\", \"index\":%d, \"total_sensors\": %d, \"temp_C\": \"%s\", \"temp_F\": \"%s\", \"error\": %d}",s_addr,i,m,"error value","error value",1);
    strcpy(outstrC,"\"error value\"");
    strcpy(outstrF,"\"error value\"");
    error = 1;
  } else {
    Homie.getLogger() << "C: " << tempC << " F: " << DallasTemperature::toFahrenheit(tempC) << endl;
    dtostrf(tempC,7,3,outstrC);
    dtostrf(DallasTemperature::toFahrenheit(tempC),7,3,outstrF);
  }
  sprintf(b,"{\"sensor\": \"%s\", \"index\":%d, \"total_sensors\": %d, \"temp_C\": %s, \"temp_F\": %s, \"error\": %d }",s_addr,i,m,outstrC,outstrF,error);
  Homie.getLogger() << "Sending: " << b << endl;
  analogNode.setProperty("temperature").send(b);
}

void loop() {
  Homie.loop();
}

