#include <stdio.h>
#include <Homie.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS_PIN 14

const int LED_PIN = 13;
const int BUTTON_PIN = 0;
const int MAX_SENSORS = 10;

int NUM_SENSORS = 0;
int numSensorsSent = 0;
const int SENSOR_INTERVAL = 10;
unsigned long lastTimeSent = 0;

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);

String lastTempSent[MAX_SENSORS];
DeviceAddress ds_addr[MAX_SENSORS];

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

    // Resolution 9 gives 0.5 degrees C accuracy. 
    // 10 is 0.25, 
    // 11 is 0.125,
    // 12 is 0.0625
    
    sensors.setResolution(ds_addr[i], 9); 
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

    if (numSensorsSent!=NUM_SENSORS) {
      analogNode.setProperty("sensor_count").send(String(NUM_SENSORS));
      Homie.getLogger() << "Sending: sensor_count:" << String(NUM_SENSORS) << endl;
    }
    
    numSensorsSent = NUM_SENSORS;
    
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

  // If same value, don't send it again.
  if (lastTempSent[i]==String(tempC,1)) {
    Homie.getLogger() << "same value, not sending." << endl;
    return;
  }

  lastTempSent[i] = String(tempC,1);
  
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

  if (tempC == -127.00) {
    Homie.getLogger() << "Error getting temperature " << endl;
    strcpy(outstrC,"\"error value\"");
    strcpy(outstrF,"\"error value\"");
    error = 1;
  } else {
    Homie.getLogger() << "C: " << tempC << " F: " << DallasTemperature::toFahrenheit(tempC) << endl;
    dtostrf(tempC,7,1,outstrC);
    dtostrf(DallasTemperature::toFahrenheit(tempC),7,1,outstrF);
  }
  sprintf(b,"{\"sensor\": \"%s\", \"index\":%d, \"total_sensors\": %d, \"temp_C\": %s, \"temp_F\": %s, \"error\": %d }",s_addr,i,m,outstrC,outstrF,error);
  Homie.getLogger() << "Sending: " << b << endl;
  analogNode.setProperty("temperature").send(b);
}

void loop() {
  Homie.loop();
}

