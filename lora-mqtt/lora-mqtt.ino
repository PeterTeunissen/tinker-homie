#include <Homie.h>
#include <stdio.h>
#include <SoftwareSerial.h>

const int NUM_DOORS = 1;
const int DEBOUNCE_DELAY = 100;  // 200 ms for debouncing
const int LED_PIN = LED_BUILTIN;
const int BUTTON_PIN = 0;

const int RX_PIN = 5; //5=D1
const int TX_PIN = 4; //4=D2
   
HomieNode loraNode("loraMqtt","loraMqtt");

volatile int ISR_Trapped = false; // Connected to D7
int isrHandled = false;
int isrStartTime = -1;

SoftwareSerial lora(RX_PIN,TX_PIN); 

bool mqttToLora(HomieRange range, String value) {
  
  String msgBuffer = "AT+SEND=0," + String(value.length()) + "," + value;
  Serial.print("Sending:");
  Serial.println(msgBuffer);
  lora.print(msgBuffer + "\r\n");
  Homie.getLogger() << "mqttToLora: " << msgBuffer << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  lora.begin(115200);
  delay(200);
  
  pinMode(LED_PIN,OUTPUT);
  
  Homie_setFirmware("mqtt-lora", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setLedPin(LED_PIN,LOW);

  loraNode.advertise("mqttToLora").settable(mqttToLora);
    
  Homie.setup();

  lora.print("AT+NETWORKID?\r\n");
  
  Homie.getLogger() << "Setup done" << endl;
}

void setupHandler() { 
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
          loraNode.setProperty("received").send(readBuffer);
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

void loopHandler() {
  loraRead();
}

void loop() {
  Homie.loop();
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
