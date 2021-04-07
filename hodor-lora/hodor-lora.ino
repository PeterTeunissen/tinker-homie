
#include <stdio.h>
#include <SoftwareSerial.h>
#include "PCF8574.h"
#include <Wire.h>

/************ Options **************************/
#define DEV_TF            0X02

const int NUM_DOORS = 1;
const int DEBOUNCE_DELAY = 100;  // 200 ms for debouncing
const int LED_PIN = LED_BUILTIN;
const int BUTTON_PIN = 0;
const int EXPANDER_ADDRESS = 0x38;

const int INT_PIN = 13;
const int RX_PIN = 5; // 5=D1 D3=0
const int TX_PIN = 4; // 4=D2 D4=2

int openPin[2] = {PIN_0,PIN_2};
int closedPin[2] = {PIN_1,PIN_3};
int relayPin[2]= {PIN_4,PIN_5};
   
int lastOpenState1 = -1;
int lastOpenState2 = -1;
int lastClosedState1 = -1;
int lastClosedState2 = -1;
int relay1On = 0;
int relay2On = 0;
unsigned long onTime;

volatile int ISR_Trapped = false; // Connected to D7
int isrHandled = true;
int isrStartTime = -1;

// Need to use 14,12 here instead of D5,D6 since the compiler does not know what D5,D6 is. 
PCF8574 expander(14,12,EXPANDER_ADDRESS);

SoftwareSerial lora(TX_PIN, RX_PIN);
  
void loraSend() {
  char buf[50];
  sprintf(buf,"[%d, %d, %d, %d, %d, %d]", lastOpenState1, lastOpenState2, lastClosedState1, lastClosedState2, relay1On, relay2On);
  String b(buf);
  String msgBuffer = "AT+SEND=0," + String(b.length()) + "," + b;
  Serial.print("Sending:");
  Serial.println(msgBuffer);
  lora.print(msgBuffer + "\r\n");
}

void loraRead() {

  String readBuffer;
  while (lora.available()) {
    int c = lora.read();
    if (c=='\n' || c=='\r') {
      // +RCV=50,5,HELLO,-99,40
      if (readBuffer.length()==0) {
        return;
      }
      Serial.print("Received:");
      Serial.println(readBuffer);
      if (readBuffer.startsWith("+RCV=")) {    
        readBuffer.remove(0,5); // remove the '+RCV=' part
        Serial.println(readBuffer);
        String msgSize = getStringPartByNr(readBuffer, ',', 1);
        String msgData = getStringPartByNr(readBuffer, ',', 2);
        Serial.println(msgSize);
        Serial.println(msgData);
        if (msgData.length()==msgSize.toInt()) {
          Serial.println("Same");
          relay1On = 0;
          relay2On = 0;
          if (msgData.indexOf("1:on")!=-1) {
            relay1On = 1;
            expander.write(relayPin[0], LOW);
            onTime = millis();
          }
          if (msgData.indexOf("2:on")!=-1) {
            relay2On = 1;
            expander.write(relayPin[1], LOW);
            onTime = millis();
          }
          Serial.print("Relay 1:");
          Serial.print(relay1On);
          Serial.print(" Relay 2:");
          Serial.println(relay2On);        
        } else {
          Serial.println("NOT Same");
        }
      }
      readBuffer="";
    } else {
      readBuffer+=char(c);
    }
  }
}

void ICACHE_RAM_ATTR expanderInterrupt(void) {    
  ISR_Trapped = true;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  lora.begin(115200);
  delay(200);

  pinMode(LED_PIN,OUTPUT);
  pinMode(INT_PIN,INPUT);

  attachInterrupt(digitalPinToInterrupt(INT_PIN), expanderInterrupt, FALLING);

  expander.begin();

  lora.print("AT+NETWORKID?\r\n");
  delay(200);
  
  Serial.println("Startup done");

  expander.write(relayPin[0], HIGH);
  expander.write(relayPin[1], HIGH);

  Serial.println("Relay init");

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

  loraRead();  
  
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

String getStringPartByNr(String data, char separator, int index) {
    int stringData = 0;        //variable to count data part nr 
    String dataPart = "";      //variable to hole the return text

    for(int i = 0; i<data.length()-1; i++) {    //Walk through the text one letter at a time
        if (data[i]==separator) {
            //Count the number of times separator character appears in the text
            stringData++;
        } else if (stringData==index) {
            //get the text when separator is the rignt one
            dataPart.concat(data[i]);
        } else if (stringData>index) {
            //return text and stop if the next separator appears - to save CPU-time
            return dataPart;
            break;
        }
    }
    //return text if this is the last part
    return dataPart;
}
