#include <SPI.h>
#include <Wire.h>  

volatile unsigned int c = 0;

void irq() {
  c++;  
}

void setup() {
  pinMode(5,INPUT); 
     
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("Serial init ok");

  attachInterrupt(digitalPinToInterrupt(5),irq,FALLING);

  delay(1500);
}

long t = millis();
unsigned int lastC=0;
void loop() {
  if (millis() - t > 1000) {
    t=millis();
    if (c!=lastC) {
      Serial.println(c);
      lastC = c;
      c = 0;
    }
  }
}
