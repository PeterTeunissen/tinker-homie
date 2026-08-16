#include <stdio.h>
#include <SoftwareSerial.h>
#include "PCF8574.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Wi-Fi Configuration
const char* ssid = "LoraHodor-";
const char* password = "password123";

/************ Options **************************/
#define DEV_TF            0X02

const int NUM_DOORS = 1;
const int DEBOUNCE_DELAY = 500;  // 200 ms for debouncing
const int LED_PIN = LED_BUILTIN;
const int BUTTON_PIN = 0;
const int EXPANDER_ADDRESS = 0x20;

int openPin[2] = {PIN_0,PIN_2};
int closedPin[2] = {PIN_1,PIN_3};
int relayPin[2]= {PIN_4,PIN_5};
   
int lastOpenState1 = -1;
int lastOpenState2 = -1;
int lastClosedState1 = -1;
int lastClosedState2 = -1;
char relay_1_val[10];
char relay_2_val[10];
int relay1On = 0;
int relay2On = 0;
unsigned long onTime;

volatile int ISR_Trapped = false; // Connected to D7
int isrHandled = true;
int isrStartTime = -1;
String gatewayAddress="";
String latestLoraMessage="";

PCF8574 expander(14,12,EXPANDER_ADDRESS);

SoftwareSerial lora(4,5);

const int INT_PIN = 13;

// Create web server instance
ESP8266WebServer server(80);

void parseAndHandleLoRaMessage(String payload) {

    // 1. Clean the string of carriage returns and newlines
  payload.trim();

  // Remove first 5 characters "+RCV="
  String rylrStr = payload.substring(5);
 
  // Message format from a Lora module is:  +RCV=44,5,hello,-6,-10
  // 44 = address of sending lora node
  // 5 = length of message
  // -6 = rssi
  // -10 = snr
  
  // 2. Locate boundaries from the front for Address
  int firstComma = rylrStr.indexOf(',');
  int secondComma = rylrStr.indexOf(',', firstComma + 1);
  
  // Extract and convert address
  int address = rylrStr.substring(firstComma + 1, secondComma).toInt();

  gatewayAddress = String(address);
  
  // 3. Locate boundaries from the back for RSSI and SNR
  int lastComma = rylrStr.lastIndexOf(',');
  int secondLastComma = rylrStr.lastIndexOf(',', lastComma - 1);

  // Extract and convert RSSI and SNR
  int rssi = rylrStr.substring(secondLastComma + 1, lastComma).toInt();
  int snr = rylrStr.substring(lastComma + 1).toInt();

  // 4. Extract everything in the middle as the message payload
  String message = rylrStr.substring(secondComma + 1, secondLastComma);

  DynamicJsonDocument json(1024);
  auto deserializeError = deserializeJson(json, message.c_str());
  serializeJson(json, Serial);

  strcpy(relay_1_val,"");
  strcpy(relay_2_val,"");
  
  if (!deserializeError) {
    Serial.println("\nParsed JSON configuration successfully.");
    if (json["relay_1"]) {
      strcpy(relay_1_val, json["relay_1"]);
    }
    if (json["relay_2"]) {
      strcpy(relay_2_val, json["relay_2"]);
    }
  }

  relay1On = 0;
  relay2On = 0;
  if (strstr(relay_1_val,"on")) {
    relay1On = 1;
    expander.write(relayPin[0], LOW);
    onTime = millis();
  }
  if (strstr(relay_2_val,"on")) {
    relay2On = 1;
    expander.write(relayPin[1], LOW);
    onTime = millis();
  }
  Serial.print("Relay 1:");
  Serial.print(relay1On);
  Serial.print(" Relay 2:");
  Serial.println(relay2On);
}

// { "source":"hodor", "open_1":1, "open_2":0, "closed_1":1, "closed_2":0, "relay_1":0, "relay_2":1 }"
// 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
//          1         2         3         4         5         6         7         8         9         10

void loraSend() {
  char buf[200];
  sprintf(buf,"{ \"source\":\"hodor\", \"open_1\":%d, \"open_2\":%d, \"closed_1\":%d, \"closed_2\":%d, \"relay_1\":%d, \"relay_2\":%d }", lastOpenState1, lastOpenState2, lastClosedState1, lastClosedState2, relay1On, relay2On);
  String snd = String(buf);
  String loraCommand = "AT+SEND=" + gatewayAddress + "," + snd.length() + "," + snd;

  Serial.print("Sending Lora:");
  Serial.println(loraCommand);

  lora.println(loraCommand); 
}

void ICACHE_RAM_ATTR expanderInterrupt(void) {    
  ISR_Trapped = true;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  lora.begin(9600);
  delay(200);

  pinMode(LED_PIN,OUTPUT);
  pinMode(INT_PIN,INPUT);

  attachInterrupt(digitalPinToInterrupt(INT_PIN), expanderInterrupt, FALLING);

  expander.begin();
  
  expander.write(relayPin[0], HIGH);
  expander.write(relayPin[1], HIGH);

  Serial.println("Relay init");

  uint8_t mac[6];
  WiFi.macAddress(mac);

  char apName[32];
  sprintf(apName, "LoraHodor_%02X%02X", mac[4], mac[5]);

  // Setup standalone Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName, password); 

  // Web server routing
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("Startup done");
}

// Build and serve the webpage with current values
void handleRoot() {
  
  String html = "<html><head><meta http-equiv='refresh' content='3'></head><body>";
  html += "<h1>ESP8266 LoRA Hodor Status</h1>";
  html += "<p><b>Current System State:</b></p>";
  html += "<p><b>Relay 1:</b> " + String(relay1On) + "</p>";
  html += "<p><b>Relay 2:</b> " + String(relay2On) + "</p>";
  html += "<p><b>Open Switch-1:</b> " + String(lastOpenState1) + "</p>";
  html += "<p><b>Open Switch-2:</b> " + String(lastOpenState2) + "</p>";
  html += "<p><b>Closed Switch-1:</b> " + String(lastClosedState1) + "</p>";
  html += "<p><b>Closed Switch-2:</b> " + String(lastClosedState2) + "</p>";
  html += "<p><b>Latest Raw LoRA Message:</b> <pre>" + latestLoraMessage + "</pre></p>";
  html += "<p><i>Page auto-refreshes every 3 seconds.</i></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void loop() {

  if (millis()-onTime> 900 && (relay1On==1 || relay2On==1)) {
    expander.write(relayPin[0], HIGH);
    expander.write(relayPin[1], HIGH);
    relay1On = 0;    
    relay2On = 0;
    Serial.println("Turning relays off");
  }
  
  if (ISR_Trapped==true) {
    ISR_Trapped = false;
    isrStartTime = millis();
    isrHandled = false;
    Serial.println("ISR timer Started");
  }

  // 1. Handle incoming web server clients
  server.handleClient();

  if (lora.available()>0) {
    
    String incomingString = lora.readStringUntil('\n');
    incomingString.trim(); // Remove extra carriage returns or spaces

    Serial.print("Received LoraMessage:");
    Serial.println(incomingString);

    latestLoraMessage = incomingString;
    
    // Check if the message starts with "+RCV="
    if (incomingString.startsWith("+RCV=")) {
      parseAndHandleLoRaMessage(incomingString);
    } else {
      Serial.println("Lora Message not an RCV message");   
    }
  }

  if (millis()-isrStartTime > DEBOUNCE_DELAY && isrHandled==false) {
    
    Serial.println("ISR Servicing");

    isrHandled = true;

    int openState1 = expander.read(openPin[0]);         
    if (openState1 != lastOpenState1) {
      lastOpenState1 = openState1;
      Serial.print("Open sensor 1 is:");
      Serial.println(openState1);
    }
  
    int openState2 = expander.read(openPin[1]);
    if (openState2 != lastOpenState2) {
      lastOpenState2 = openState2;
      Serial.print("Open sensor 2 is:");
      Serial.println(openState2);
    }
  
    int closedState1 = expander.read(closedPin[0]);    
    if (closedState1 != lastClosedState1) {
      lastClosedState1 = closedState1;
      Serial.print("Closed sensor 1 is:");
      Serial.println(closedState1);
    }
  
    int closedState2 = expander.read(closedPin[1]);
    if (closedState2 != lastClosedState2) {
      lastClosedState2 = closedState2;
      Serial.print("Closed sensor 2 is:");
      Serial.println(closedState2);
    }

    loraSend();
  }
}
