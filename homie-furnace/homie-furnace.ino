#include <stdio.h>
#include <Homie.h>

const int SEND_INTERVAL = 60;
const int LED_TIME = 500;
unsigned long DEBOUNCE_DELAY = 50;
const int LED_PIN = 2;
const int BUTTON_PIN = 0;

const int W_PIN = 12;
const int W2_PIN = 14;
const int Y_PIN = 4;
const int G_PIN = 5;

unsigned long ledOn = 0;

struct ButtonState {
  int pin;
  char name[5];
  unsigned long lastTimeSent = 0;
  int currentState;
  int debounceStart;
  int debounceState;
};

HomieNode hvacNode("hvac", "hvac");

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  pinMode(W_PIN,INPUT_PULLUP);
  pinMode(W2_PIN,INPUT_PULLUP);
  pinMode(Y_PIN,INPUT_PULLUP);
  pinMode(G_PIN,INPUT_PULLUP);

  digitalWrite(LED_PIN,HIGH);

  Homie_setFirmware("homie-hvac", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setLedPin(LED_PIN,LOW).setResetTrigger(BUTTON_PIN, LOW, 5000);
  hvacNode.advertise("W");
  hvacNode.advertise("W2");
  hvacNode.advertise("Y");
  hvacNode.advertise("G");
  
  Homie.setup();
  Homie.getLogger() << "Setup done" << endl;
}

ButtonState buttons[4];

void setupHandler() {
  strcpy(buttons[0].name,"W");
  buttons[0].pin = W_PIN;
  buttons[0].currentState = digitalRead(buttons[0].pin);
  buttons[0].debounceState = buttons[0].currentState;
  buttons[0].debounceStart = millis();
  buttons[0].lastTimeSent = 0;
    
  strcpy(buttons[1].name,"W2");
  buttons[1].pin = W2_PIN;
  buttons[1].currentState = digitalRead(buttons[1].pin);
  buttons[1].debounceState = buttons[1].currentState;
  buttons[1].debounceStart = millis();
  buttons[1].lastTimeSent = 0;
  
  strcpy(buttons[2].name,"Y");
  buttons[2].pin = Y_PIN;
  buttons[2].currentState = digitalRead(buttons[2].pin);
  buttons[2].debounceState = buttons[2].currentState;
  buttons[2].debounceStart = millis();
  buttons[2].lastTimeSent = 0;
  
  strcpy(buttons[3].name,"G");
  buttons[3].pin = G_PIN;
  buttons[3].currentState = digitalRead(buttons[3].pin);
  buttons[3].debounceState = buttons[3].currentState;
  buttons[3].debounceStart = millis();
  buttons[3].lastTimeSent = 0;
}

void loopHandler() {       
  for(byte i=0;i<4;i++) {
    monitorButton(&buttons[i]);
  }
}

void monitorButton(ButtonState *state) {

  int reading = digitalRead(state->pin);

  if (reading != state->currentState && reading != state->debounceState) {
    state->debounceStart = millis();
    state->debounceState = reading;
  }

  if (((millis() - state->debounceStart) > DEBOUNCE_DELAY) && (reading==state->debounceState)) {
    if (reading != state->currentState) {
      state->currentState = reading;
      sendState(state);
      state->lastTimeSent = millis();
    }
  }
  
  if (millis() - state->lastTimeSent > SEND_INTERVAL * 1000UL || state->lastTimeSent == 0) {
    sendState(state);
    state->lastTimeSent = millis();
  }

  if (millis()-ledOn  > LED_TIMEL) {  
    digitalWrite(LED_PIN,HIGH);
  }
}

void sendState(ButtonState *state) {
  char b[100];
  sprintf(b,"{\"signal\": \"%s\", \"pin\":%d, \"value\":%d }",state->name,state->pin,state->currentState);
  Homie.getLogger() << "Sending: " << b << endl;
  hvacNode.setProperty(String(state->name)).send(b);  
  digitalWrite(LED_PIN,LOW);
  ledOn = millis();
}

void loop() {
  Homie.loop();
}

