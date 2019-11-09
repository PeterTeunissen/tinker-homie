#include <Wire.h>  
#include "SSD1306.h" 
#include <WiFi.h>
#include "PubSubClient.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "PCF8574.h"
#include <stdlib.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#include <ConnectionInfo.h>
#include "HealthHandler.h"
#include "LevelSensors.h"
#include "FlowSensor.h"
#include "PumpHandler.h"
#include "ValveHandler.h"
#include "AlertHandler.h"
#include "Zone.h"
#include "Scheduler.h"
#include "TempSensor.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// I2C devices
SSD1306 display(0x3c, 4, 15);
PCF8574 expander(4, 15, 0x38);

#define MQTT_BOOT_TOPIC    "/homie/irrigation/boot/"
#define MQTT_BASE_TOPIC    "/homie/irrigation/vals/"
#define MQTT_RECEIVE_TOPIC "/homie/irrigation/actions"

#define SDA_PIN     4
#define SCLK_PIN    15
#define FLOW_PIN    35
#define PUMP_PIN    22
#define TEMP_PIN    17

#define VALVE_PIN_A 13
#define VALVE_PIN_B 21
#define VALVE_PIN_C 12
#define VALVE_PIN_D 25

#define LINE_HEIGHT 12

#define EXPANDER_IRQ_PIN 23

#define TEMP_INTERVAL                 10000
#define LOW_FLOW_GRACE_MILLI_SECONDS  2000
#define FLOW_MINIMUM                  30

OneWire oneWire(TEMP_PIN); 
DallasTemperature dallasSensors(&oneWire);

Scheduler *scheduler;
PumpHandler *pumpHandler;
FlowSensor *flowSensor;
LevelSensors *levelSensors;
HealthHandler *healthHandler;
ValveHandler *valveHandler;
AlertHandler *alertHandler;
TempSensor *tempSensor;

Zone *zoneA;
Zone *zoneB;
Zone *zoneC;
Zone *zoneD;

boolean g_refreshDisplay=false;
char g_temp[8];

void tempCallBack(float ftemp) {
  char temp[8];
  sprintf(temp,"%.1f",ftemp);

  if (strcmp(temp, g_temp)!=0) {
    publishMQTTData("temp", temp);
    sprintf(g_temp,"%sF", temp);  
  }
}

void flowCallBack(unsigned int flow) {

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(BLACK);
  display.fillRect(98, 24, 28, LINE_HEIGHT);
  display.setColor(WHITE);
  display.drawString(98, 24, ": " + String(flow));

  char val[10];
  sprintf(val, "%d", flow);
  publishMQTTData("flow", val);
      
  g_refreshDisplay = true;
}

char g_progressChar;

void alertCallBack(boolean isRunning, int errNo, char *msg) {

  // Poor mans spinning progress character
  char tkn[] = {'|','/','-','\\','|','/','-','\\'};
  int w = 4000;
  int c = w/sizeof(tkn);
  
  char t=' ';
  div_t sdiv = div(millis(), w);
  div_t divresult = div(sdiv.rem, c);
  if (isRunning) {
    t = tkn[divresult.quot % sizeof(tkn)];
  }
  
  if (t!=g_progressChar) {
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setColor(BLACK);
    display.fillRect(115, 12, 13, LINE_HEIGHT);
    display.setColor(WHITE);
    display.drawString(128, 12, String(t));
    g_progressChar = t;
    g_refreshDisplay = true;
  }
    
  if (strlen(msg)!=0) {
    char val[30];
    sprintf(val, "%d-%s", errNo, msg);
    publishMQTTData("alert", val);  
  }
}

void levelCallBack(unsigned int level) {

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(BLACK);
  display.fillRect(32, 12, 60, LINE_HEIGHT);
  display.setColor(WHITE);
  display.drawString(32, 12, ": " + String(level) +" % Full");

  char val[10];
  sprintf(val, "%d", level);
  publishMQTTData("tankLevel", val);
      
  g_refreshDisplay = true;
}

void pumpCallBack(boolean isOn) {

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(BLACK);
  display.fillRect(32, 24, 20, LINE_HEIGHT);
  display.setColor(WHITE);
  display.drawString(32, 24, ": " + (isOn? String("On"):String("Off"))) ;

  char val[10];
  sprintf(val,"%s", isOn ? "true" : "false");
  publishMQTTData("pumpOn", val);

  g_refreshDisplay = true;
}

void valveCallBack(unsigned int valve, boolean isOn, int timeLeft) {

  int x = 0;
  int y = 0;

  if (valve==1) {
    x = 32;
    y = 36;
  }
  if (valve==2) {
    x = 32;
    y = 48;
  }
  if (valve==3) {
    x = 98;
    y = 36;
  }
  if (valve==4) {
    x = 98;
    y = 48;
  }
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(BLACK);
  display.fillRect(x, y, 28, LINE_HEIGHT);
  display.setColor(WHITE);
  display.drawString(x, y, ": " + (isOn ? String(timeLeft) + " s" : String("Off"))) ;

  char topic[30];
  char val[10];

  sprintf(topic,"zone/%d/on", valve);  
  sprintf(val, "%s", isOn ? "true" : "false");
  publishMQTTData(topic, val);

  sprintf(topic, "zone/%d/timeLeft", valve);
  sprintf(val, "%d", timeLeft);
  publishMQTTData(topic, val);

  g_refreshDisplay = true;
}

void secondCallBack() {
  String formattedTime = timeClient.getFormattedTime();

  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setColor(BLACK);
  display.fillRect(70, 0, 128-70, LINE_HEIGHT);
  display.setColor(WHITE);

  if ((timeClient.getSeconds() % 10) < 5 ) {
    display.drawString(128, 0, g_temp);
  } else {
    display.drawString(128, 0, formattedTime.c_str());
  }
  
  if (timeClient.getSeconds() % 10 == 0) {
    char val[10];
    sprintf(val, "%s", formattedTime.c_str());
    publishMQTTData("slaveTime", val);
  }
  
  g_refreshDisplay = true;
}

void healthCallBack(unsigned int upTime, unsigned int rssi) {

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(BLACK);
  display.fillRect(32, 0, 32+20, LINE_HEIGHT);
  display.setColor(WHITE);
  display.drawString(32, 0, ": " + String(rssi));

  char val[10];
  sprintf(val,"%d", rssi);
  publishMQTTData("rssi", val);

  sprintf(val, "%d", upTime);
  publishMQTTData("uptime", val);

  g_refreshDisplay = true;
}

void flowTick() {
  flowSensor->handleIRQ();  
}

void expanderIRQ() {
  levelSensors->handleIRQ();
}

void setupWifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Irrigator-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(MQTT_BOOT_TOPIC, "Irrigation System reboot");
      // ... and subscribe
      client.subscribe(MQTT_RECEIVE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttCallBack(char* topic, byte *payload, unsigned int length) {
  Serial.println("-------new message from broker-----");
  Serial.print("channel:");
  Serial.println(topic);
  Serial.print("data:");  
  Serial.write(payload, length);
  Serial.println();
  
  String message;
  
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (message=="STOP") {
    scheduler->stopSchedule();
  } 

  if (message.startsWith("SCHEDULE:")) {
    message = message.substring(9);
    Serial.print("Left:");
    Serial.println(message);

    char buf[message.length()+1];
    message.toCharArray(buf, sizeof(buf));

    Serial.println(buf);
    
    char * pch;
    pch = strtok(buf, ",");
    int v = 0;
    int times[4] = {0,0,0,0};
    while (pch != NULL) {
      v++;
      int i = atoi(pch);
      times[v-1]=i;
      Serial.print("Valve:");
      Serial.print(v);
      Serial.print(" time:");
      Serial.println(i);
      pch = strtok(NULL, ",");
    }
    scheduler->scheduleZones(times);    
  }
}

void publishMQTTData(char *topic, char *serialData){

  if (!client.connected()) {
    reconnectMQTT();
  }

  char mqttTopic[100];
  sprintf(mqttTopic, "%s%s", MQTT_BASE_TOPIC, topic);

  client.publish(mqttTopic, serialData, true);
}

void setup() {
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high、

  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("Setup Display");

  Serial.println();
  Serial.println("init ok");

  setupWifi();
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallBack);
  reconnectMQTT();  

  Serial.println("mqtt init ok");

  timeClient.begin();
  timeClient.setTimeOffset(-4*3600);

  pinMode(EXPANDER_IRQ_PIN, INPUT);
  pinMode(FLOW_PIN,INPUT);
  
  pinMode(VALVE_PIN_A, OUTPUT);
  pinMode(VALVE_PIN_B, OUTPUT);
  pinMode(VALVE_PIN_C, OUTPUT);
  pinMode(VALVE_PIN_D, OUTPUT);

  pinMode(PUMP_PIN, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowTick, FALLING);
  attachInterrupt(digitalPinToInterrupt(EXPANDER_IRQ_PIN), expanderIRQ, FALLING);
  
  pumpHandler = new PumpHandler(PUMP_PIN, pumpCallBack);
  flowSensor = new FlowSensor(flowCallBack);
  levelSensors = new LevelSensors(&expander, levelCallBack);
  healthHandler = new HealthHandler(healthCallBack, secondCallBack, &timeClient);
  valveHandler = new ValveHandler(1,VALVE_PIN_A, valveCallBack);
  alertHandler = new AlertHandler(alertCallBack);
  tempSensor = new TempSensor(tempCallBack, &dallasSensors, TEMP_INTERVAL);
  
  zoneA = new Zone(new ValveHandler(1, VALVE_PIN_A, valveCallBack), pumpHandler, levelSensors, flowSensor, alertHandler, LOW_FLOW_GRACE_MILLI_SECONDS, FLOW_MINIMUM);
  zoneB = new Zone(new ValveHandler(2, VALVE_PIN_B, valveCallBack), pumpHandler, levelSensors, flowSensor, alertHandler, LOW_FLOW_GRACE_MILLI_SECONDS, FLOW_MINIMUM);
  zoneC = new Zone(new ValveHandler(3, VALVE_PIN_C, valveCallBack), pumpHandler, levelSensors, flowSensor, alertHandler, LOW_FLOW_GRACE_MILLI_SECONDS, FLOW_MINIMUM);
  zoneD = new Zone(new ValveHandler(4, VALVE_PIN_D, valveCallBack), pumpHandler, levelSensors, flowSensor, alertHandler, LOW_FLOW_GRACE_MILLI_SECONDS, FLOW_MINIMUM);

  scheduler = new Scheduler(alertHandler);
  scheduler->addZone(zoneA);
  scheduler->addZone(zoneB);
  scheduler->addZone(zoneC);
  scheduler->addZone(zoneD);

  display.init();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  initLabels();

  // Now have to set the I2C bus speed back to 100khz, since the PCF8574 can not run at the high speed that the SSD1306 can handle.
  // (The PCF8574 library sets the speed to 400k)
  // The place of this line in the code makes a difference! Needs to be after the display.init() and before the expander.begin()!!
  Wire.setClock(100000);
  
  expander.begin();

  dallasSensors.begin();
  
  delay(1500);

  scheduler->init();
}

void initLabels() {

  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "RSSI");

  display.setTextAlignment(TEXT_ALIGN_RIGHT);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 12, "Tank");
  
  display.drawString(0, 24, "Pump");

  display.drawString(66, 24, "Flow");

  display.drawString(0, 36, "ZoneA");

  display.drawString(66, 36, "ZoneC");

  display.drawString(0, 48, "ZoneB");

  display.drawString(66, 48, "ZoneD");

  display.display();
}

void loop() {
  
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  flowSensor->loop();
  healthHandler->loop();
  levelSensors->loop();
  pumpHandler->loop();
  tempSensor->loop();
  scheduler->loop();
  
  if (g_refreshDisplay) {
    display.display();
    g_refreshDisplay = false;
  }
}
