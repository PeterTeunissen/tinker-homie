#include <SPI.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include "images.h"
#include <WiFi.h>
#include "PubSubClient.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi network name and password:
const char* ssid = "1465WL";
const char* password = "6109170381";
const char* mqtt_server = "192.168.1.25";

#define mqtt_port 1883
#define MQTT_USER "switches"
#define MQTT_PASSWORD "bitches"
#define MQTT_SERIAL_PUBLISH_CH "/homie/pump/rssi"
#define MQTT_SERIAL_RECEIVER_CH "/homie/pump/zone"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

SSD1306 display(0x3c, 4,15);
uint8_t rssi;
int i = 0;

void loraData(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0 , 15 , "RSSI: "+ String(rssi) + " %");
  display.drawString(0 , 26 , "Loop:" + String(i));
 // display.drawStringMaxWidth(0 , 26 , 128, packet);
 // display.drawString(0, 0, rssi); 
  display.display();
  Serial.println(i);
}

uint8_t rssiToPercentage(int32_t rssi) {
  uint8_t quality;
  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }

  return quality;
}

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("/homie/pump/status", "hello world");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
}

void setup() {
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high、
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("Serial init ok");

  display.init();
  //display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);

  Serial.println("Display init ok");

  Serial.println("Doing Wifi.begin...");

  setup_wifi();

  Serial.println("Wifi init ok");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();  

  Serial.println("mqtt init ok");

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-4*3600);
  
  delay(1500);
}

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

long t = millis();
int lastSec=0;
void loop() {
  if (millis() - t > 5000) {
    char buf[30];
    rssi = rssiToPercentage(WiFi.RSSI());
    i++;
    sprintf(buf,"%d-%d",rssi,i);
    publishSerialData(buf);
    t=millis();
    //loraData();
  }
   while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  if (timeClient.getSeconds() != lastSec) {
    String formattedTime = timeClient.getFormattedTime();
    Serial.println(formattedTime);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setColor(BLACK);
    display.fillRect(70,0,128-70,10);
    display.setColor(WHITE);
    display.drawString(128, 0, formattedTime.c_str());
    display.display();
    lastSec=timeClient.getSeconds();
  }  
}
