#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

#include "RylrLink.h"

#define LORA_DEBUG 1

// SoftwareSerial: RX = digital pin 2, TX = digital pin 3
SoftwareSerial loraSerial(D5,D6); 
RylrLink loraLink(loraSerial);

// Wi-Fi Configuration
const char* ssid = "LoraSwitch-";
const char* password = "password123";
const int LED_ON = LOW;
const int LED_OFF = HIGH;
const int RELAY_ON = HIGH;
const int RELAY_OFF = LOW;
const int LED_1 = D1;
const int LED_2 = D2;

// Pin Configurations
const int LED_PIN = LED_BUILTIN;    // Built-in LED or external LED
const int RELAY_PIN = D7;  // Relay control pin

// Global states to track output conditions and messaging
bool systemState = false; 
String latestLoraMessage = "No message received yet."; 

int ledStatus = 0;
int m_lastSnr=0;
int m_lastRssi=0;

// Create web server instance
ESP8266WebServer server(80);

// CALLBACK 1: Handles incoming structural text data payload messages
void handleCustomData(int senderID, String message, int rssi, int snr) {
  Serial.print("\n[Data Callback] Received from ");
  Serial.print(senderID);
  Serial.print(": ");
  Serial.print(message);
  Serial.print(" rssi(raw): ");
  Serial.print(rssi);
  Serial.print(" snr: ");
  Serial.println(snr);

  latestLoraMessage = message;
  m_lastSnr = snr;
  m_lastRssi = rssi;

  if (message.indexOf("ON") >= 0) {
    systemState = true;
    digitalWrite(LED_PIN, LED_ON);
    digitalWrite(RELAY_PIN, RELAY_ON);
    ledStatus++;
  } 
  else if (message.indexOf("OFF") >= 0) {
    systemState = false;
    digitalWrite(LED_PIN, LED_OFF);
    digitalWrite(RELAY_PIN, RELAY_OFF);
    ledStatus++;
  } else {
    ledStatus=0;
  }

}

// CALLBACK 2: Updates whenever the library state switches
void handleStateChange(bool led1State, bool led2State){
  digitalWrite(LED_1, led1State ? HIGH : LOW);
  digitalWrite(LED_2, led2State ? HIGH : LOW);
}

//Build and serve the webpage with current values
void handleRoot() {
  String stateString = systemState ? "HIGH (ON)" : "LOW (OFF)";
  
  String html = "<html><head><meta http-equiv='refresh' content='3'></head><body>";
  html += "<h1>ESP8266 LoRA & Relay Status</h1>";
  html += "<p><b>Current System State:</b> " + stateString + "</p>";
  html += "<p><b>LED Pin (D1):</b> " + stateString + "</p>";
  html += "<p><b>Relay Pin (D2):</b> " + stateString + "</p>";
  html += "<p><b>Latest Raw LoRA Message:</b> <pre>" + latestLoraMessage + "</pre></p>";
  html += "<p><b>Latest RSSI:</b>" + String(m_lastRssi) + "</p>";
  html += "<p><b>Latest SNR:</b> " + String(m_lastSnr) + "</p>";
  html += "<p><i>Page auto-refreshes every 3 seconds.</i></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  // Initialize Native Hardware Serial for RYLR LoRA module (Default 115200)
  Serial.begin(9600);
  loraSerial.begin(9600);
  delay(1000);
  
  // Initialize Pins
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  // Ensure starting state is off
  digitalWrite(LED_PIN, LED_OFF);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  uint8_t mac[6];
  WiFi.macAddress(mac);

  char apName[32];
  sprintf(apName, "LoraSwitch_%02X%02X", mac[4], mac[5]);

  // Setup standalone Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName, password); 

  // Web server routing
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  // Initialize library with callbacks: dataCallback, stateCallback, maxRetries, retryInterval
  loraLink.begin(handleCustomData, handleStateChange, 5, 3000);

  Serial.println("Started...");
}

boolean d = false;
String loraData="";

void ledShow() {

  if (ledStatus>2) {
    ledStatus=1;
  }

  if (ledStatus==0) {
    digitalWrite(LED_1, LOW);    
    digitalWrite(LED_2, LOW);    
  }

  if (ledStatus==1) {
    digitalWrite(LED_1, HIGH);    
    digitalWrite(LED_2, LOW);    
  }

  if (ledStatus==2) {
    digitalWrite(LED_1, LOW);    
    digitalWrite(LED_2, HIGH);    
  }
}

void loop() {
  // 1. Handle incoming web server clients
  server.handleClient();

  loraLink.update();
  
//  d=false;
//  loraData="";
//    
//  if (loraSerial.available()>0) {
//    loraData = loraSerial.readStringUntil('\n');
//    Serial.print("softwareSerial message:");
//    Serial.println(loraData);
//    d=true;
//  }
//
//  if (Serial.available()>0) {
//    loraData = Serial.readStringUntil('\n');
//    Serial.print("Serial message:");
//    Serial.println(loraData);
//    d=true;
//  }
//
//
//  if (d) {
//    loraData.trim(); // Remove whitespace or newlines
//    // Save the exact raw message to display on the webpage
//    latestLoraMessage = loraData;
//    
//    // Check string for content keywords
//    if (loraData.indexOf("ON") >= 0) {
//      systemState = true;
//      digitalWrite(LED_PIN, LED_ON);
//      digitalWrite(RELAY_PIN, RELAY_ON);
//      ledStatus++;
//    } 
//    else if (loraData.indexOf("OFF") >= 0) {
//      systemState = false;
//      digitalWrite(LED_PIN, LED_OFF);
//      digitalWrite(RELAY_PIN, RELAY_OFF);
//      ledStatus++;
//    } else {
//      ledStatus=0;
//    }
//  }

  ledShow();
}
