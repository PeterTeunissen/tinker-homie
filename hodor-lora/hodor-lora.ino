//#include <Homie.h>

#include <stdio.h>
#include "PCF8574.h"
#include <Wire.h>

/************ Options **************************/
#define DEV_TF            0X02

const int NUM_DOORS = 1;
const int DEBOUNCE_DELAY = 100;  // 200 ms for debouncing
const int LED_PIN = LED_BUILTIN;
const int BUTTON_PIN = 0;
const int EXPANDER_ADDRESS = 0x20;

const int INT_PIN = 13;
const int RX_PIN = 0;
const int TX_PIN = 2;

int openPin[2] = {PIN_0,PIN_2};
int closedPin[2] = {PIN_1,PIN_3};
int relayPin[2]= {PIN_4,PIN_5};
   
//HomieNode switches1("switches1","switches1");
//HomieNode switches2("switches2","switches2");

//HomieNode relay1("relay1","relay1");
//HomieNode relay2("relay2","relay2");

int lastOpenState1 = -1;
int lastOpenState2 = -1;
int lastClosedState1 = -1;
int lastClosedState2 = -1;
int relay1On = 0;
int relay2On = 0;
unsigned long onTime;

volatile int ISR_Trapped = false; // Connected to D7
int isrHandled = false;
int isrStartTime = -1;

// Need to use 14,12 here instead of D5,D6 since the compiler does not know what D5,D6 is. 
PCF8574 expander(14,12,EXPANDER_ADDRESS);

//bool relay1OnHandler(HomieRange range, String value) {
//  if (value != "on" && value != "off") return false;
//
//  bool on = (value == "on");
//  expander.write(relayPin[0], on ? LOW : HIGH);
//  relay1.setProperty("activate").send(value);
//  Homie.getLogger() << "Relay 1 is " << (on ? "on" : "off") << endl;
//
//  return true;
//}

//bool relay2OnHandler(HomieRange range, String value) {
//  if (value != "on" && value != "off") return false;
//
//  bool on = (value == "on");
//  expander.write(relayPin[1], on ? LOW : HIGH);
//  relay2.setProperty("activate").send(value);
//  Homie.getLogger() << "Relay 2 is " << (on ? "on" : "off") << endl;
//
//  return true;
//}

void loraSend() {
  char buf[50];
  sprintf(buf,"[%d, %d, %d, %d, %d, %d]", lastOpenState1, lastOpenState2, lastClosedState1, lastClosedState2, relay1On, relay2On);
  String b(buf);
  Serial.print("AT+SEND=0," + String(b.length()) + b + "\n\r");
}

void loraRead() {

  String readBuffer;
  while (Serial.available()) {
    int c = Serial.read();
    if (c=='\n' || c=='\r') {
      // +RCV=50,5,HELLO,-99,40
      readBuffer.remove(0,5); // remove the '+RCV=' part
      String msgSize = getStringPartByNr(readBuffer, ',', 1);
      String msgData = getStringPartByNr(readBuffer, ',', 2);
      if (msgData.length()==msgSize.toInt()) {
        String r1 = getStringPartByNr(msgData, ' ', 1);
        relay1On = 0;
        relay2On = 0;
        if (r1=="on") {
          relay1On = 1;
          expander.write(relayPin[0], LOW);
          onTime = millis();
        }
        String r2 = getStringPartByNr(msgData, ' ', 2);
        if (r2=="on") {
          relay2On = 1;
          expander.write(relayPin[1], LOW);
          onTime = millis();
        }
      }
    } else {
      readBuffer+=char(c);
    }
  }
}

void expanderInterrupt(void) {    
  ISR_Trapped = true;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN,OUTPUT);

  expander.begin();

  attachInterrupt(digitalPinToInterrupt(INT_PIN), expanderInterrupt, FALLING);
  
//  Homie_setFirmware("homie-hodor", "1.0.0");
//  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
//  Homie.setLedPin(LED_PIN,LOW);//setResetTrigger(BUTTON_PIN, LOW, 5000);

//  relay1.advertise("activate").settable(relay1OnHandler);
//  relay2.advertise("activate").settable(relay2OnHandler);
    
//  Homie.setup();
//  Homie.getLogger() << "Setup done" << endl;
}


void setupHandler() { 

}

void loopHandler() {

  if ((millis()-onTime> 200) && (relay1On==1 || relay2On==1)) {
    expander.write(relayPin[0], HIGH);
    expander.write(relayPin[1], HIGH);
    relay1On = 0;    
    relay2On = 0;    
  }
  
  if (ISR_Trapped==true) {
    ISR_Trapped = false;
    isrStartTime = millis();
    isrHandled = false;
//    Homie.getLogger() << "ISR timer Started" << endl;
  }

  if (Serial.available()) {
    loraRead();  
  }
  
  if (millis()-isrStartTime > DEBOUNCE_DELAY && isrHandled==false) {
//    Homie.getLogger() << "ISR Servicing" << endl;

    isrHandled = true;

    int openState1 = expander.read(openPin[0]);         
    if (openState1 != lastOpenState1) {
      lastOpenState1 = openState1;
//      switches1.setProperty("open").send(openState1>0? "false" : "true" );
//      Homie.getLogger() << "Open sensor 1 is " << (openState1>0? "false" : "true") << endl;
    }
  
    int openState2 = expander.read(openPin[1]);
    if (openState2 != lastOpenState2) {
      lastOpenState2 = openState2;
//      switches2.setProperty("open").send(openState2>0? "false" : "true" );
//      Homie.getLogger() << "Open sensor 2 is " << (openState2>0? "false" : "true") << endl;
    }
  
    int closedState1 = expander.read(closedPin[0]);    
    if (closedState1 != lastClosedState1) {
      lastClosedState1 = closedState1;
//      switches1.setProperty("closed").send(closedState1>0? "false" : "true" );
//      Homie.getLogger() << "Closed sensor 1 is " << (closedState1>0? "false" : "true") << endl;
    }
  
    int closedState2 = expander.read(closedPin[1]);
    if (closedState2 != lastClosedState2) {
      lastClosedState2 = closedState2;
//      switches2.setProperty("closed").send(closedState2>0? "false" : "true" );
//      Homie.getLogger() << "Closed sensor 2 is " << (closedState2>0? "false" : "true") << endl;
    }

    loraSend();
  }
}


void loop() {
//  Homie.loop();
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
