#include <stdio.h>
#include <Homie.h>
#include <SoftwareSerial.h>

#define CMD_PLAY_W_INDEX 0X03
#define CMD_SET_VOLUME 0X06
#define CMD_SEL_DEV 0X09
#define DEV_TF 0X02
#define CMD_PLAY 0X0D
#define CMD_PAUSE 0X0E
#define CMD_SINGLE_CYCLE 0X19
#define SINGLE_CYCLE_ON 0X00
#define SINGLE_CYCLE_OFF 0X01
#define CMD_PLAY_W_VOL 0X22

const int NUM_DOORS = 2;
const int DEBOUNCE_DELAY = 50;  // 50 ms for debouncing
const int LED_PIN = 13;
const int BUTTON_PIN = 0;

int relayPin[2]= {4,5};
int openPin[2] = {12,14};
int closedPin[2] = {10,11};

Bounce openDebouncer1 = Bounce();
Bounce openDebouncer2 = Bounce();
Bounce closeDebouncer1 = Bounce();  
Bounce closeDebouncer2 = Bounce();  
    
HomieNode relay1("hodor","relay/1");
HomieNode relay2("hodor","relay/2");

HomieNode switches1("hodor","switches/1");
HomieNode switches2("hodor","switches/2");

int lastOpenState1 = -1;
int lastOpenState2 = -1;
int lastClosedState1 = -1;
int lastClosedState2 = -1;

SoftwareSerial mp3(5,6);

HomieNode sound("hodor", "sound");

bool soundOnHandler(HomieRange range, String value) {

  sound.setProperty("play").send(value);
  Homie.getLogger() << "Play track " << value << endl;
  return true;
}

bool relay1OnHandler(HomieRange range, String value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(relayPin[0], on ? HIGH : LOW);
  relay1.setProperty("active").send(value);
  Homie.getLogger() << "Relay 1 is " << (on ? "on" : "off") << endl;

  return true;
}

bool relay2OnHandler(HomieRange range, String value) {
  if (value != "true" && value != "false") return false;

  bool on = (value == "true");
  digitalWrite(relayPin[1], on ? HIGH : LOW);
  relay2.setProperty("active").send(value);
  Homie.getLogger() << "Relay 2 is " << (on ? "on" : "off") << endl;

  return true;
}

void setup() {
  Serial.begin(115200);
  mp3.begin(9600);
  delay(500);
  mp3Command(CMD_SEL_DEV, DEV_TF);//select the TF card  
  delay(200);
  mp3Command(CMD_PLAY_W_VOL, 0X0F01);//play the first song with volume 15 class
  
  pinMode(LED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  digitalWrite(LED_PIN,HIGH);

  Homie_setFirmware("homie-hodor", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setLedPin(LED_PIN,LOW).setResetTrigger(BUTTON_PIN, LOW, 5000);

  relay1.advertise("activate").settable(relay1OnHandler);
  relay2.advertise("activate").settable(relay2OnHandler);
  sound.advertise("play").settable(soundOnHandler);
    
  Homie.setup();
  Homie.getLogger() << "Setup done" << endl;
}


void setupHandler() {
  pinMode(openPin[0],INPUT);
  openDebouncer1.attach(openPin[0]);
  openDebouncer1.interval(DEBOUNCE_DELAY);

  pinMode(openPin[1],INPUT);
  openDebouncer2.attach(openPin[1]);
  openDebouncer2.interval(DEBOUNCE_DELAY);

  pinMode(closedPin[0],INPUT);
  closeDebouncer1.attach(closedPin[0]);
  closeDebouncer1.interval(DEBOUNCE_DELAY);

  pinMode(closedPin[1],INPUT);
  closeDebouncer2.attach(closedPin[1]);
  closeDebouncer2.interval(DEBOUNCE_DELAY);

  pinMode(relayPin[0],OUTPUT);
  pinMode(relayPin[1],OUTPUT);

  digitalWrite(relayPin[0],LOW);
  digitalWrite(relayPin[1],LOW);
}

void loopHandler() {
  int openState1 = openDebouncer1.read();
  if (openState1 != lastOpenState1) {
    lastOpenState1 = openState1;
    switches1.setProperty("open").send(openState1>0? "true" : "false" );
  }

  int openState2 = openDebouncer2.read();
  if (openState2 != lastOpenState2) {
    lastOpenState2 = openState2;
    switches2.setProperty("open").send(openState2>0? "true" : "false" );
  }

  int closedState1 = closeDebouncer1.read();
  if (closedState1 != lastClosedState1) {
    lastClosedState1 = closedState1;
    switches1.setProperty("closed").send(closedState1>0? "true" : "false" );
  }

  int closedState2 = closeDebouncer2.read();
  if (closedState2 != lastClosedState2) {
    lastClosedState2 = closedState2;
    switches2.setProperty("closed").send(closedState2>0? "true" : "false" );
  }
}


void loop() {
  Homie.loop();
  openDebouncer1.update();
  openDebouncer2.update();
  closeDebouncer1.update();
  closeDebouncer2.update();
}

void mp3Command(int8_t command, int16_t dat) {

  uint8_t cmdBuf[8];

  delay(20);

  cmdBuf[0] = 0x7e; //starting byte
  cmdBuf[1] = 0xff; //version
  cmdBuf[2] = 0x06; //the number of bytes of the command without starting byte and ending byte
  cmdBuf[3] = command; //
  cmdBuf[4] = 0x00;//0x00 = no feedback, 0x01 = feedback
  cmdBuf[5] = (int8_t)(dat >> 8);//datah
  cmdBuf[6] = (int8_t)(dat); //datal
  cmdBuf[7] = 0xef; //ending byte
  for(uint8_t i=0; i<8; i++) {
    mp3.write(cmdBuf[i]);
  }
}

