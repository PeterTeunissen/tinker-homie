
#define ESP32 1
#define LED_BUILTIN 27

#include <Homie.h>

#include <stdio.h>
#include <HardwareSerial.h>
#include "PCF8574.h"
#include <Wire.h>

// ---- MP3 Player commands ---

#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY     0X16
#define CMD_FOLDER_CYCLE  0X17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON  0X00
#define DAC_OFF 0X01

#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C
#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_VOLUME      0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS  0x48
#define CMD_QUERY_FLDR_COUNT  0x4f

/************ Options **************************/
#define DEV_TF            0X02

const int NUM_DOORS = 1;
const int DEBOUNCE_DELAY = 100;  // 200 ms for debouncing
const int LED_PIN = LED_BUILTIN;
const int BUTTON_PIN = 11;
const int EXPANDER_ADDRESS = 0x20;

const int INT_PIN = 13;
const int RX_PIN = 0;
const int TX_PIN = 2;

int openPin[2] = {PIN_0,PIN_2};
int closedPin[2] = {PIN_1,PIN_3};
int relayPin[2]= {PIN_4,PIN_5};
   
HomieNode switches1("switches1","switches1");
HomieNode switches2("switches2","switches2");

HomieNode relay1("relay1","relay1");
HomieNode relay2("relay2","relay2");

HomieNode sound("sound", "sound");

int lastOpenState1 = -1;
int lastOpenState2 = -1;
int lastClosedState1 = -1;
int lastClosedState2 = -1;

volatile int ISR_Trapped = false; // Connected to D7
int isrHandled = false;
int isrStartTime = -1;

// Need to use 14,12 here instead of D5,D6 since the compiler does not know what D5,D6 is. 
PCF8574 expander(14,12,EXPANDER_ADDRESS);

//HardwareSerial mp3(1);

bool soundOnHandler(HomieRange range, String value) {
  Homie.getLogger() << "Sound command raw:" << value  << endl;

  String sCommand = getStringPartByNr(value, '-', 0);
  String sOption = getStringPartByNr(value, '-', 1);

  Homie.getLogger() << "Sound sCommand:" << sCommand << ", sOption:" << sOption << endl;

  int8_t command = sCommand.toInt();
  int16_t option = sOption.toInt(); 

  //mp3Command(command,option);

  sound.setProperty("play").send(value);
  Homie.getLogger() << "Sound command: " << command << ", option: " << option << endl;
  return true;
}

bool relay1OnHandler(HomieRange range, String value) {
  if (value != "on" && value != "off") return false;

  bool on = (value == "on");
  expander.write(relayPin[0], on ? LOW : HIGH);
  relay1.setProperty("activate").send(value);
  Homie.getLogger() << "Relay 1 is " << (on ? "on" : "off") << endl;

  return true;
}

bool relay2OnHandler(HomieRange range, String value) {
  if (value != "on" && value != "off") return false;

  bool on = (value == "on");
  expander.write(relayPin[1], on ? LOW : HIGH);
  relay2.setProperty("activate").send(value);
  Homie.getLogger() << "Relay 2 is " << (on ? "on" : "off") << endl;

  return true;
}

void expanderInterrupt(void) {    
  ISR_Trapped = true;
}

void setup() {
  Serial.begin(115200);
  //mp3.begin(9600,SERIAL_8N1, RX_PIN, TX_PIN );
  delay(1000);
  //mp3Command(CMD_SEL_DEV, DEV_TF);
  delay(1000);
  //mp3Command(CMD_SET_VOLUME, 30);
  delay(1000);
  
  pinMode(LED_PIN,OUTPUT);

  expander.begin();

  attachInterrupt(digitalPinToInterrupt(INT_PIN), expanderInterrupt, FALLING);
  
  Homie_setFirmware("homie-hodor", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setLedPin(LED_PIN,LOW);//setResetTrigger(BUTTON_PIN, LOW, 5000);

  relay1.advertise("activate").settable(relay1OnHandler);
  relay2.advertise("activate").settable(relay2OnHandler);
  sound.advertise("play").settable(soundOnHandler);
    
  Homie.setup();
  Homie.getLogger() << "Setup done" << endl;
}


void setupHandler() { 

}

void loopHandler() {

//  if (mp3.available()) {
//    String mp3Response = decodeMP3Answer();
//    sound.setProperty("response").send(mp3Response);
//    Homie.getLogger() << "Sound response: " << mp3Response << endl;
//  }
  
  if (ISR_Trapped==true) {
    ISR_Trapped = false;
    isrStartTime = millis();
    isrHandled = false;
    Homie.getLogger() << "ISR timer Started" << endl;
  }

  if (millis()-isrStartTime > DEBOUNCE_DELAY && isrHandled==false) {
    Homie.getLogger() << "ISR Servicing" << endl;

    isrHandled = true;

    int openState1 = expander.read(openPin[0]);         
    if (openState1 != lastOpenState1) {
      lastOpenState1 = openState1;
      switches1.setProperty("open").send(openState1>0? "false" : "true" );
      Homie.getLogger() << "Open sensor 1 is " << (openState1>0? "false" : "true") << endl;
    }
  
    int openState2 = expander.read(openPin[1]);
    if (openState2 != lastOpenState2) {
      lastOpenState2 = openState2;
      switches2.setProperty("open").send(openState2>0? "false" : "true" );
      Homie.getLogger() << "Open sensor 2 is " << (openState2>0? "false" : "true") << endl;
    }
  
    int closedState1 = expander.read(closedPin[0]);    
    if (closedState1 != lastClosedState1) {
      lastClosedState1 = closedState1;
      switches1.setProperty("closed").send(closedState1>0? "false" : "true" );
      Homie.getLogger() << "Closed sensor 1 is " << (closedState1>0? "false" : "true") << endl;
    }
  
    int closedState2 = expander.read(closedPin[1]);
    if (closedState2 != lastClosedState2) {
      lastClosedState2 = closedState2;
      switches2.setProperty("closed").send(closedState2>0? "false" : "true" );
      Homie.getLogger() << "Closed sensor 2 is " << (closedState2>0? "false" : "true") << endl;
    }
  }
}


void loop() {
  Homie.loop();
}



/********************************************************************************/
/*Function: sbyte2hex. Returns a byte data in HEX format.                       */
/*Parameter:- uint8_t b. Byte to convert to HEX.                                */
/*Return: String                                                                */
/********************************************************************************/
String sbyte2hex(uint8_t b) {
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
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
