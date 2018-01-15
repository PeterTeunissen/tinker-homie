#include <stdio.h>
#include <Homie.h>

const int NUM_PINS = 4;
const int SEND_INTERVAL = 60;             // Send the state every 60 seconds anyway
const unsigned long DEBOUNCE_DELAY = 50;  // 50 ms for debouncing
const int LED_TIME = 500;                 // Blue led turns on 500ms for every mqtt send
const int LED_PIN = 2;                    // Blue led is in pin 2
const int BUTTON_PIN = 0;                 // Reset button is on pin 0

const int W_PIN = 12;
const int W2_PIN = 14;
const int Y_PIN = 4;
const int G_PIN = 5;

unsigned long ledOn = 0;      // Holds the time the led was turned on

int pins[NUM_PINS] = {
  W_PIN,W2_PIN,Y_PIN,G_PIN
};

char names[5][NUM_PINS] = {"W","W2","Y","G"};

struct ButtonState {        // Info for one input
  int pin;
  char name[5];
  unsigned long lastTimeSent = 0;
  int currentState;
  unsigned long debounceStart;
  int debounceState;
};

HomieNode hvacNode("hvac", "hvac");

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  for(int i=0;i<NUM_PINS;i++) {
    pinMode(pins[i],INPUT_PULLUP); 
  }
  digitalWrite(LED_PIN,HIGH);

  Homie_setFirmware("homie-hvac", "1.0.0");
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setLedPin(LED_PIN,LOW).setResetTrigger(BUTTON_PIN, LOW, 5000);
  for(int i=0;i<NUM_PINS;i++) {  
    hvacNode.advertise(names[i]);
  }  
  Homie.setup();
  Homie.getLogger() << "Setup done" << endl;
}

ButtonState buttons[4];

void setupHandler() {
  for(int i=0;i<NUM_PINS;i++) {
    initButton(pins[i],names[i],&buttons[i]);    
  }
}

void initButton(int pin, char *name, ButtonState *state) {
  state->pin = pin;
  strcpy(state->name,name);
  state->currentState = digitalRead(state->pin);
  state->debounceState = state->currentState;
  state->debounceStart = millis();
  state->lastTimeSent = 0;
}

void loopHandler() {       
  for(byte i=0;i<NUM_PINS;i++) {
    monitorButton(&buttons[i]);
  }
}

void monitorButton(ButtonState *state) {
  
  int reading = digitalRead(state->pin);

  if (reading != state->currentState && reading != state->debounceState) {
    state->debounceStart = millis();
    state->debounceState = reading;
  }

  // Check the state after DEBOUNCE_DELAY and send an update if it's stable.
  if (((millis() - state->debounceStart) > DEBOUNCE_DELAY) && (reading==state->debounceState)) {
    if (reading != state->currentState) {
      state->currentState = reading;
      sendState(state);
      state->lastTimeSent = millis();
    }
  }

  // Force the send of the state every SEND_INTERVAL seconds, just so the host has the current values.
  if (millis() - state->lastTimeSent > SEND_INTERVAL * 1000UL || state->lastTimeSent == 0) {
    sendState(state);
    state->lastTimeSent = millis();
  }

  // Turn LED off after time expires
  if (millis()-ledOn  > LED_TIME) {  
    digitalWrite(LED_PIN,HIGH);
  }
}

void sendState(ButtonState *state) {
  char b[100];
  // Make a JSON message with some info.
  sprintf(b,"{\"signal\": \"%s\", \"pin\":%d, \"value\":%d }",state->name,state->pin,state->currentState);
  Homie.getLogger() << "Sending: " << b << endl;
  hvacNode.setProperty(String(state->name)).send(b);  

  // Turn on the LED to indicate we sent something.
  digitalWrite(LED_PIN,LOW);
  // Start the LED timer,
  ledOn = millis();
}

void loop() {
  Homie.loop();
}

